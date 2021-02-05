"""
Copyright (c) Microsoft Corporation. All rights reserved.
Licensed under the MIT license.
"""

from __future__ import print_function
from __future__ import unicode_literals
import os
import sys
import ctypes
import binascii
import manifest_types
import manifest_common
import manifest_parser
from Crypto.PublicKey import RSA


PCD_CONFIG_FILENAME = "pcd_generator.config"


class pcd_port (ctypes.LittleEndianStructure):
    _pack_ = 1
    _fields_ = [('port_id', ctypes.c_ubyte),
                ('port_flags', ctypes.c_ubyte),
                ('policy', ctypes.c_ubyte),
                ('reserved', ctypes.c_ubyte),
    			('spi_frequency_hz', ctypes.c_uint)]

class pcd_mux (ctypes.LittleEndianStructure):
    _pack_ = 1
    _fields_ = [('mux_address', ctypes.c_ubyte),
                ('mux_channel', ctypes.c_ubyte),
                ('reserved', ctypes.c_ushort)]


def generate_ports_buf (xml_ports):
    """
    Create a buffer of pcd_port struct instances from parsed XML list

    :param xml_ports: List of parsed XML of ports to be included in PCD

    :return Ports buffer, number of ports
    """

    if xml_ports is None or len (xml_ports) < 1:
        return None, 0

    num_ports = len (xml_ports)
    ports_buf = (ctypes.c_ubyte * (ctypes.sizeof (pcd_port) * num_ports)) ()
    ports_len = 0

    for id, port in xml_ports.items ():
        spifreq = int (manifest_common.get_key_from_dict (port, "spifreq", "Port"))
        resetctrl = int (manifest_common.get_key_from_dict (port, "resetctrl", "Port"))
        flashmode = int (manifest_common.get_key_from_dict (port, "flashmode", "Port"))
        policy = int (manifest_common.get_key_from_dict (port, "policy", "Port"))

        portflags = (flashmode << 2) | resetctrl 

        port_body = pcd_port (int (id), portflags, policy, 0, spifreq)

        ctypes.memmove (ctypes.addressof (ports_buf) + ports_len, ctypes.addressof (port_body),
            ctypes.sizeof (pcd_port))
        ports_len += ctypes.sizeof (pcd_port)

    return ports_buf, num_ports

def generate_rot (xml_rot, num_components, hash_engine):
    """
    Create an RoT object from parsed XML list and ports buffer

    :param xml_rot: List of parsed XML of RoT to be included in RoT object
    :param num_components: Number of components
    :param hash_engine: Hashing engine

    :return Instance of an RoT object, RoT's TOC entry, RoT hash
    """

    rottype = int (manifest_common.get_key_from_dict (xml_rot, "type", "RoT"))
    rotaddress = int (manifest_common.get_key_from_dict (xml_rot["interface"], "address", 
        "RoT interface"))
    roteid = int (manifest_common.get_key_from_dict (xml_rot["interface"], "roteid", 
        "RoT interface"))
    bridgeeid = int (manifest_common.get_key_from_dict (xml_rot["interface"], "bridgeeid", 
        "RoT interface"))
    bridgeaddress = int (manifest_common.get_key_from_dict (xml_rot["interface"], "bridgeaddress", 
        "RoT interface"))

    rotflags = rottype

    if "ports" in xml_rot:
        ports, num_ports = generate_ports_buf (xml_rot["ports"])
    else:
        ports = (ctypes.c_ubyte * 0) ()
        num_ports = 0

    class pcd_rot_element (ctypes.LittleEndianStructure):
        _pack_ = 1
        _fields_ = [('rot_flags', ctypes.c_ubyte),
                    ('port_count', ctypes.c_ubyte),
                    ('components_count', ctypes.c_ubyte),
                    ('rot_address', ctypes.c_ubyte),
                    ('rot_eid', ctypes.c_ubyte),
                    ('bridge_address', ctypes.c_ubyte),
                    ('bridge_eid', ctypes.c_ubyte),
                    ('reserved', ctypes.c_ubyte),
                    ('ports', ctypes.c_ubyte * ctypes.sizeof (ports))]

    rot = pcd_rot_element (rotflags, num_ports, num_components, rotaddress, roteid, bridgeaddress, 
        bridgeeid, 0, ports)
    rot_len = ctypes.sizeof (rot)
    rot_toc_entry = manifest_common.manifest_toc_entry (manifest_common.PCD_V2_ROT_TYPE_ID, 
        manifest_common.V2_BASE_TYPE_ID, 1, 0, 0, rot_len)

    rot_hash = manifest_common.generate_hash (rot, hash_engine)

    return rot, rot_toc_entry, rot_hash

def generate_muxes_buf (xml_muxes):
    """
    Create a buffer of pcd_mux struct instances from parsed XML list

    :param xml_muxes: List of parsed XML of muxes to be included in PCD

    :return Muxes buffer, length of muxes buffer, number of muxes
    """

    if xml_muxes is None or len (xml_muxes) < 1:
        return None, 0

    num_muxes = len (xml_muxes)
    muxes_buf = (ctypes.c_ubyte * (ctypes.sizeof (pcd_mux) * num_muxes)) ()
    muxes_len = 0

    for mux in sorted (xml_muxes.items ()):
        address = int (manifest_common.get_key_from_dict (mux[1], "address", "Mux"))
        channel = int (manifest_common.get_key_from_dict (mux[1], "channel", "Mux"))

        mux_body = pcd_mux (address, channel, 0)

        ctypes.memmove (ctypes.addressof (muxes_buf) + muxes_len, ctypes.addressof (mux_body),
            ctypes.sizeof (pcd_mux))
        muxes_len += ctypes.sizeof (pcd_mux)

    return muxes_buf, muxes_len, num_muxes

def generate_cpld (xml_cpld, hash_engine):
    """
    Create a CPLD object from parsed XML list

    :param xml_cpld: List of parsed XML of CPLD to be included in CPLD object
    :param hash_engine: Hashing engine

    :return Instance of a CPLD object, CPLD's TOC entry, hash of CPLD object
    """

    if xml_cpld["interface"]["type"] is not 0:
        raise ValueError ("Unsupported CPLD interface type: {0}".format (
            xml_cpld["interface"]["type"]))

    if "muxes" in xml_cpld["interface"]:
        muxes, muxes_len, num_muxes = generate_muxes_buf (xml_cpld["interface"]["muxes"])
    else:
        muxes = (ctypes.c_ubyte * 0)()
        muxes_len = 0
        num_muxes = 0

    bus = int (manifest_common.get_key_from_dict (xml_cpld["interface"], "bus", "CPLD interface"))
    address = int (manifest_common.get_key_from_dict (xml_cpld["interface"], "address", 
        "CPLD interface"))
    eid = int (manifest_common.get_key_from_dict (xml_cpld["interface"], "eid", 
        "CPLD interface"))
    i2cmode = int (manifest_common.get_key_from_dict (xml_cpld["interface"], "i2cmode", 
        "CPLD interface"))

    i2c_flags = i2cmode 

    class pcd_cpld_element (ctypes.LittleEndianStructure):
        _pack_ = 1
        _fields_ = [('mux_count', ctypes.c_ubyte, 4),
                    ('i2c_flags', ctypes.c_ubyte, 4),
                    ('bus', ctypes.c_ubyte),
                    ('address', ctypes.c_ubyte),
                    ('eid', ctypes.c_ubyte),
                    ('muxes', ctypes.c_ubyte * muxes_len)]

    cpld = pcd_cpld_element (num_muxes, i2c_flags, bus, address, eid, muxes)
    cpld_len = ctypes.sizeof (cpld)
    cpld_toc_entry = manifest_common.manifest_toc_entry (manifest_common.PCD_V2_I2C_CPLD_TYPE_ID, 
        manifest_common.V2_BASE_TYPE_ID, 1, 0, 0, cpld_len)

    cpld_hash = manifest_common.generate_hash (cpld, hash_engine)

    return cpld, cpld_toc_entry, cpld_hash

def generate_direct_component_buf (xml_component):
    """
    Create a direct component object from parsed XML list

    :param xml_component: List of parsed XML of component to be included in direct component object

    :return Instance of a component object, component's TOC entry, component hash
    """

    if xml_component["interface"]["type"] is not 0:
        raise ValueError ("Unsupported direct component interface type: {0}".format (
            xml_component["interface"]["type"]))

    policy = int (manifest_common.get_key_from_dict (xml_component, "policy", "Direct Component"))
    powerctrl_reg = int (manifest_common.get_key_from_dict (xml_component["powerctrl"], "register",
        "Direct Component"))
    powerctrl_mask = int (manifest_common.get_key_from_dict (xml_component["powerctrl"], "mask",
        "Direct Component"))
    component_type = manifest_common.get_key_from_dict (xml_component, "type", "Direct Component")
    i2cmode = int (manifest_common.get_key_from_dict (xml_component["interface"], "i2cmode", 
        "Direct Component"))
    bus = int (manifest_common.get_key_from_dict (xml_component["interface"], "bus", 
        "Direct Component"))
    address = int (manifest_common.get_key_from_dict (xml_component["interface"], "address", 
        "Direct Component"))
    eid = int (manifest_common.get_key_from_dict (xml_component["interface"], "eid", 
        "Direct Component"))

    type_len = len (component_type)
    i2c_flags = i2cmode 

    padding_len = ((type_len + 3) & (~3)) - type_len
    padding = (ctypes.c_ubyte * padding_len) ()
    ctypes.memset (padding, 0, ctypes.sizeof (ctypes.c_ubyte) * padding_len)

    if "muxes" in xml_component["interface"]:
        muxes, muxes_len, num_muxes = generate_muxes_buf (xml_component["interface"]["muxes"])
    else:
        muxes = (ctypes.c_ubyte * 0) ()
        muxes_len = 0
        num_muxes = 0

    class pcd_direct_i2c_component_element (ctypes.LittleEndianStructure):
        _pack_ = 1
        _fields_ = [('policy', ctypes.c_ubyte),
                    ('power_ctrl_reg', ctypes.c_ubyte),
                    ('power_ctrl_mask', ctypes.c_ubyte),
                    ('type_len', ctypes.c_ubyte),
                    ('type', ctypes.c_char * type_len),
                    ('type_padding', ctypes.c_ubyte * padding_len),
                    ('mux_count', ctypes.c_ubyte, 4),
                    ('i2c_flags', ctypes.c_ubyte, 4),
                    ('bus', ctypes.c_ubyte),
                    ('address', ctypes.c_ubyte),
                    ('eid', ctypes.c_ubyte),
                    ('muxes', ctypes.c_ubyte * muxes_len)]

    component = pcd_direct_i2c_component_element (policy, powerctrl_reg, powerctrl_mask, type_len, 
        component_type.encode ('utf-8'), padding, num_muxes, i2c_flags, bus, address, eid, muxes)
    component_len = ctypes.sizeof (component)

    component_toc_entry = manifest_common.manifest_toc_entry (
        manifest_common.PCD_V2_DIRECT_COMPONENT_TYPE_ID, manifest_common.V2_BASE_TYPE_ID, 1, 
        0, 0, component_len)

    component_hash = manifest_common.generate_hash (component, hash_engine)

    return component, component_toc_entry, component_hash

def generate_mctp_bridge_component_buf (xml_component):
    """
    Create an MCTP bridges component object from parsed XML list

    :param xml_component: List of parsed XML of component to be included in MCTP bridge component 
        object

    :return Instance of a component object, component's TOC entry, component hash
    """

    policy = int (manifest_common.get_key_from_dict (xml_component, "policy", 
        "MCTP Bridge Component"))
    powerctrl_reg = int (manifest_common.get_key_from_dict (xml_component["powerctrl"], "register",
        "MCTP Bridge Component"))
    powerctrl_mask = int (manifest_common.get_key_from_dict (xml_component["powerctrl"], "mask",
        "MCTP Bridge Component"))
    component_type = manifest_common.get_key_from_dict (xml_component, "type", 
        "MCTP Bridge Component")
    device_id = int (manifest_common.get_key_from_dict (xml_component, "deviceid", 
        "MCTP Bridge Component"))
    vendor_id = int (manifest_common.get_key_from_dict (xml_component, "vendorid", 
        "MCTP Bridge Component"))
    sub_device_id = int (manifest_common.get_key_from_dict (xml_component, "subdeviceid", 
        "MCTP Bridge Component"))
    sub_vendor_id = int (manifest_common.get_key_from_dict (xml_component, "subvendorid", 
        "MCTP Bridge Component"))
    sub_vendor_id = int (manifest_common.get_key_from_dict (xml_component, "subvendorid", 
        "MCTP Bridge Component"))
    components_count = int (manifest_common.get_key_from_dict (xml_component, "count", 
        "MCTP Bridge Component"))
    eid = int (manifest_common.get_key_from_dict (xml_component, "eid", "MCTP Bridge Component"))

    type_len = len (component_type)

    padding_len = ((type_len + 3) & (~3)) - type_len
    padding = (ctypes.c_ubyte * padding_len) ()
    ctypes.memset (padding, 0, ctypes.sizeof (ctypes.c_ubyte) * padding_len)

    class pcd_mctp_bridge_component_element (ctypes.LittleEndianStructure):
        _pack_ = 1
        _fields_ = [('policy', ctypes.c_ubyte),
                    ('power_ctrl_reg', ctypes.c_ubyte),
                    ('power_ctrl_mask', ctypes.c_ubyte),
                    ('type_len', ctypes.c_ubyte),
                    ('type', ctypes.c_char * type_len),
                    ('type_padding', ctypes.c_ubyte * padding_len),
                    ('device_id', ctypes.c_ushort),
                    ('vendor_id', ctypes.c_ushort),
                    ('subsystem_device_id', ctypes.c_ushort),
                    ('subsystem_vendor_id', ctypes.c_ushort),
                    ('components_count', ctypes.c_ubyte),
                    ('eid', ctypes.c_ubyte),
                    ('reserved', ctypes.c_ushort)]

    component = pcd_mctp_bridge_component_element (policy, powerctrl_reg, powerctrl_mask, type_len, 
        component_type.encode ('utf-8'), padding, device_id, vendor_id, sub_device_id, 
        sub_vendor_id, components_count, eid, 0)
    component_len = ctypes.sizeof (component)

    component_toc_entry = manifest_common.manifest_toc_entry (
        manifest_common.PCD_V2_MCTP_BRIDGE_COMPONENT_TYPE_ID, manifest_common.V2_BASE_TYPE_ID, 1, 
        0, 0, component_len)

    component_hash = manifest_common.generate_hash (component, hash_engine)

    return component, component_toc_entry, component_hash

def generate_components (xml_components, hash_engine):
    """
    Create a buffer of component section struct instances from parsed XML list

    :param xml_components: List of parsed XML of components to be included in PCD
    :param hash_engine: Hashing engine

    :return Components buffer, number of components, list of component TOC entries, 
        list of component hashes
    """

    if xml_components is None or len (xml_components) < 1:
        return None, 0, 0

    components_list = []
    components_toc_list = []
    hash_list = []
    components_len = 0

    for component in xml_components:
        connection = manifest_common.get_key_from_dict (component, "connection", "Component")

        if connection is manifest_parser.PCD_COMPONENT_CONNECTION_DIRECT:
            component_buf, component_toc_entry, component_hash = generate_direct_component_buf (
                component)

        elif connection is manifest_parser.PCD_COMPONENT_CONNECTION_MCTP_BRIDGE:
            component_buf, component_toc_entry, component_hash = \
                generate_mctp_bridge_component_buf (component)

        else:
            raise ValueError ("Unsupported component connection type: {0}".format (connection))
        
        components_list.append (component_buf)
        components_toc_list.append (component_toc_entry)
        hash_list.append (component_hash)

        components_len += ctypes.sizeof (component_buf)

    components_buf = (ctypes.c_ubyte * components_len) ()
    offset = 0

    for component in components_list:
        component_len = ctypes.sizeof (component)
        ctypes.memmove (ctypes.addressof (components_buf) + offset, ctypes.addressof (component),
            component_len)

        offset += component_len

    return components_buf, len (xml_components), components_toc_list, hash_list


#*************************************** Start of Script ***************************************

if len (sys.argv) < 2:
    path = os.path.join (os.path.dirname (os.path.abspath (__file__)), PCD_CONFIG_FILENAME)
else:
    path = os.path.abspath (sys.argv[1])

processed_xml, sign, key_size, key, key_type, hash_type, pcd_id, output, xml_version = \
    manifest_common.load_xmls (path, 1, manifest_types.PCD)

hash_engine = manifest_common.get_hash_engine (hash_type)

processed_xml = list (processed_xml.items())[0][1]

num_components = 0
pcd_len = 0
elements_list = []
toc_list = []
hash_list = []

manifest_header = manifest_common.generate_manifest_header (pcd_id, key_size, manifest_types.PCD, 
    hash_type, key_type, xml_version)
manifest_header_len = ctypes.sizeof (manifest_header)
pcd_len += manifest_header_len

platform_id, platform_id_toc_entry, platform_id_hash = manifest_common.generate_platform_id_buf (
    processed_xml, hash_engine)
    
pcd_len += ctypes.sizeof (platform_id)
elements_list.append (platform_id)
toc_list.append (platform_id_toc_entry)
hash_list.append (platform_id_hash)

if "cpld" in processed_xml:
    cpld, cpld_toc_entry, cpld_hash = generate_cpld (processed_xml["cpld"], hash_engine)
    
    pcd_len += ctypes.sizeof (cpld)
    elements_list.append (cpld)
    toc_list.append (cpld_toc_entry)
    hash_list.append (cpld_hash)

if "components" in processed_xml:
    components, num_components, components_toc_list, components_hash_list = generate_components (
        processed_xml["components"], hash_engine)

    pcd_len += ctypes.sizeof (components)
    elements_list.append (components)
    toc_list.extend (components_toc_list)
    hash_list.extend (components_hash_list)

rot, rot_toc_entry, rot_hash = generate_rot (processed_xml["rot"], num_components, hash_engine)
    
pcd_len += ctypes.sizeof (rot)
elements_list.append (rot)
toc_list.append (rot_toc_entry)
hash_list.append (rot_hash)

toc = manifest_common.generate_toc (hash_engine, hash_type, toc_list, hash_list)
toc_len = ctypes.sizeof (toc)
pcd_len += toc_len

manifest_header.length = pcd_len + manifest_header.sig_length

pcd_buf = (ctypes.c_ubyte * pcd_len) ()
offset = 0

ctypes.memmove (ctypes.addressof (pcd_buf) + offset, ctypes.addressof (manifest_header), 
    manifest_header_len)
offset += manifest_header_len

ctypes.memmove (ctypes.addressof (pcd_buf) + offset, ctypes.addressof (toc), toc_len)
offset += toc_len

for element in elements_list:
    element_len = ctypes.sizeof (element)
    ctypes.memmove (ctypes.addressof (pcd_buf) + offset, ctypes.addressof (element), element_len)

    offset += element_len

manifest_common.write_manifest (xml_version, sign, pcd_buf, key, key_size, key_type, output, 
    manifest_header.length - manifest_header.sig_length, manifest_header.sig_length)

print ("Completed PCD generation: {0}".format (output))


