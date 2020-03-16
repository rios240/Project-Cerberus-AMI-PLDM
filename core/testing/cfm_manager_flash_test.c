// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "testing.h"
#include "manifest/cfm/cfm_manager_flash.h"
#include "state_manager/system_state_manager.h"
#include "mock/flash_master_mock.h"
#include "mock/cfm_observer_mock.h"
#include "mock/signature_verification_mock.h"
#include "engines/hash_testing_engine.h"
#include "flash/flash_common.h"
#include "crypto/ecc.h"
#include "cfm_testing.h"


static const char *SUITE = "cfm_manager_flash";


/**
 * CFM with ID 2 for testing.
 */
const uint8_t CFM2_DATA[] = {
	0xbc,0x01,0x92,0xa5,0x02,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0xb0,0x00,0x02,0x00,
	0x40,0x00,0x01,0x00,0x04,0x00,0x00,0x00,0x38,0x00,0x01,0x00,0x08,0x00,0x00,0x00,
	0x76,0x34,0x2e,0x30,0x34,0x2e,0x30,0x34,0x28,0x00,0x20,0x00,0x03,0x00,0x00,0x00,
	0x85,0x08,0xf3,0x46,0xb4,0xda,0x1f,0xec,0x3e,0x78,0x20,0xc1,0x58,0x2f,0x73,0xe2,
	0x1c,0x18,0xa2,0x83,0x5d,0xc0,0x99,0x26,0x0b,0xb9,0xaf,0x13,0x65,0x03,0xee,0x2d,
	0x6c,0x00,0x01,0x00,0x03,0x00,0x00,0x00,0x64,0x00,0x02,0x00,0x09,0x00,0x00,0x00,
	0x76,0x30,0x33,0x2e,0x30,0x33,0x2e,0x30,0x33,0x00,0x00,0x00,0x28,0x00,0x20,0x00,
	0x02,0x00,0x00,0x00,0x85,0x08,0xf3,0x46,0xb4,0xda,0x1f,0xec,0x3e,0x78,0x20,0xc1,
	0x58,0x2f,0x73,0xe2,0x1c,0x18,0xa2,0x83,0x5d,0xc0,0x99,0x26,0x0b,0xb9,0xaf,0x13,
	0x65,0x03,0xee,0x2d,0x28,0x00,0x20,0x00,0x03,0x00,0x00,0x00,0x85,0x08,0xf3,0x46,
	0xb4,0xda,0x1f,0xec,0x3e,0x78,0x20,0xc1,0x58,0x2f,0x73,0xe2,0x1c,0x18,0xa2,0x83,
	0x5d,0xc0,0x99,0x26,0x0b,0xb9,0xaf,0x13,0x65,0x03,0xee,0x2d,0xb6,0x8a,0xd2,0xf2,
	0x88,0xd7,0xd2,0xa6,0xeb,0x4c,0x48,0x57,0x73,0x32,0x26,0x41,0xf4,0x52,0x58,0x68,
	0xa3,0x4d,0x2f,0xd7,0xac,0x2b,0x4f,0x1e,0xd7,0x29,0xf1,0x8f,0x10,0x0b,0x1d,0xca,
	0x40,0x7a,0xf5,0xa5,0xe0,0xf0,0x11,0x37,0x14,0xd0,0x76,0xbe,0x7e,0x4b,0x6e,0x75,
	0xb5,0x61,0x7a,0x8c,0xc0,0xa1,0xc7,0x87,0x14,0x83,0x84,0x60,0xe1,0x97,0x5d,0xa2,
	0x74,0x26,0x23,0xec,0x12,0xe7,0x40,0x9f,0xa5,0xe9,0x9e,0xae,0xa5,0x60,0xc9,0x24,
	0x24,0x67,0x85,0x7d,0x94,0xd2,0xf7,0x62,0x79,0xbe,0xb0,0xf6,0x50,0xdc,0x4f,0xac,
	0x65,0xd1,0x6c,0x45,0xc0,0x66,0x89,0x57,0x82,0x1e,0x94,0x1f,0x33,0x5f,0x87,0x5a,
	0x9b,0xc6,0x64,0xad,0xe0,0xbf,0x19,0x07,0xdf,0xf5,0x78,0x03,0x8e,0x87,0x3d,0xb1,
	0x86,0x3d,0x06,0xf9,0xc9,0xaf,0x19,0x62,0x90,0x5b,0xee,0x67,0x02,0xa5,0x75,0xe6,
	0x2a,0x83,0x27,0x26,0x13,0xc0,0x31,0xf2,0x7d,0x7c,0xff,0x2e,0x23,0x79,0x53,0x16,
	0x8e,0xf3,0x2f,0x05,0xc4,0x47,0xe9,0xa9,0x3e,0x0a,0xc6,0xde,0x4c,0x97,0x40,0x8f,
	0x55,0x16,0x66,0x0f,0xca,0xd2,0x43,0x06,0xb4,0xf0,0x32,0x7e,0x9e,0x8c,0x7d,0x77,
	0x26,0x4f,0x7c,0xad,0x90,0x13,0x35,0x8e,0xd4,0x65,0x7b,0xb4,0x7c,0x32,0x7a,0x32,
	0x2a,0xc0,0x07,0x2f,0x22,0x9a,0x60,0x54,0xc6,0x8a,0xd3,0x31,0x68,0x85,0xcf,0xf5,
	0x09,0x4d,0xa0,0x2f,0x6a,0x5c,0x36,0x0d,0xf6,0x93,0x3f,0x6d,0x74,0x29,0x60,0xc6,
	0x30,0xf7,0x6d,0xb8,0x4d,0x6e,0xa6,0x53,0x47,0x84,0x3c,0xc1
};

/**
 * Length of the second testing CFM.
 */
const uint32_t CFM2_DATA_LEN = sizeof (CFM2_DATA);

/**
 * The offset from the base for the second CFM signature.
 */
const uint32_t CFM2_SIGNATURE_OFFSET = (sizeof (CFM2_DATA) - 256);

/**
 * The signature for the second CFM.
 */
const uint8_t *CFM2_SIGNATURE = CFM2_DATA + (sizeof (CFM2_DATA) - 256);

/**
 * The length of the CFM signature.
 */
const size_t CFM2_SIGNATURE_LEN = 256;

/**
 * CFM2_DATA hash for testing.
 */
const uint8_t CFM2_HASH[] = {
	0x8c,0x0e,0xbf,0x80,0x79,0x71,0x39,0xf1,0x08,0x4f,0xad,0x8c,0xd0,0x6b,0x92,0xfb,
	0x6a,0xb0,0x9b,0xac,0x06,0x09,0x74,0x14,0xd9,0xa7,0xcb,0x1f,0x67,0x66,0x0a,0x8d
};


/**
 * Initialize the system state manager for testing.
 *
 * @param test The testing framework.
 * @param state The system state instance to initialize.
 * @param flash_mock The mock for the flash state storage.
 * @param flash The flash device to initialize for state.
 */
static void cfm_manager_flash_testing_init_system_state (CuTest *test,
	struct state_manager *state, struct flash_master_mock *flash_mock, struct spi_flash *flash)
{
	int status;
	uint16_t end[4] = {0xffff, 0xffff, 0xffff, 0xffff};

	status = flash_master_mock_init (flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (flash, &flash_mock->base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_expect_rx_xfer (flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (flash_mock, 0, (uint8_t*) end, sizeof (end),
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, 8));

	status |= flash_master_mock_expect_rx_xfer (flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (flash_mock, 0, (uint8_t*) end, sizeof (end),
		FLASH_EXP_READ_CMD (0x03, 0x11000, 0, -1, 8));

	status |= flash_master_mock_expect_erase_flash_sector_verify (flash_mock, 0x10000, 0x1000);

	CuAssertIntEquals (test, 0, status);

	status = system_state_manager_init (state, &flash->base, 0x10000);
	CuAssertIntEquals (test, 0, status);
}

/**
 * Set up expectations for verifying a CFM on flash.
 *
 * @param flash_mock The mock for the CFM flash storage.
 * @param verification The mock for CFM verification.
 * @param pfm The CFM data to read.
 * @param length The length of the CFM data.
 * @param hash The CFM hash.
 * @param signature The CFM signature.
 * @param offset The offset of the CFM signature.
 * @param address The base address of the CFM.
 *
 * @return 0 if the expectations were set up successfully or an error code.
 */
static int cfm_manager_flash_testing_verify_cfm (struct flash_master_mock *flash_mock,
	struct signature_verification_mock *verification, const uint8_t *cfm, size_t length,
	const uint8_t *hash, const uint8_t *signature, size_t offset, uint32_t address)
{
	int status;

	status = flash_master_mock_expect_rx_xfer (flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (flash_mock, 0, cfm, length,
		FLASH_EXP_READ_CMD (0x03, address, 0, -1, CFM_HEADER_SIZE));

	status |= flash_master_mock_expect_rx_xfer (flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (flash_mock, 0, signature, CFM_SIGNATURE_LEN,
		FLASH_EXP_READ_CMD (0x03, address + offset, 0, -1, CFM_SIGNATURE_LEN));

	status |= flash_master_mock_expect_verify_flash (flash_mock, address, cfm,
		length - CFM_SIGNATURE_LEN);

	status |= mock_expect (&verification->mock, verification->base.verify_signature, verification,
		0, MOCK_ARG_PTR_CONTAINS (hash, CFM_HASH_LEN), MOCK_ARG (CFM_HASH_LEN),
		MOCK_ARG_PTR_CONTAINS (signature, CFM_SIGNATURE_LEN), MOCK_ARG (CFM_SIGNATURE_LEN));

	return status;
}

/**
 * Write complete CFM data to the manager to enable pending CFM verification.
 *
 * @param test The test framework.
 * @param manager The manager to use for writing CFM data.
 * @param flash_mock The mock for CFM flash storage.
 * @param addr The expected address of CFM writes.
 *
 * @return The number of CFM bytes written.
 */
static int cfm_manager_flash_testing_write_new_cfm (CuTest *test, struct cfm_manager_flash *manager,
	struct flash_master_mock *flash_mock, uint32_t addr)
{
	int status;
	uint8_t data[] = {0x01, 0x02, 0x03, 0x04};

	status = flash_master_mock_expect_erase_flash_verify (flash_mock, addr, 0x10000);
	status |= flash_master_mock_expect_write_ext (flash_mock, addr, data, sizeof (data), true, 0);

	CuAssertIntEquals (test, 0, status);

	status = manager->base.base.clear_pending_region (&manager->base.base, sizeof (data));
	CuAssertIntEquals (test, 0, status);

	status = manager->base.base.write_pending_data (&manager->base.base, data, sizeof (data));
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock->mock);
	CuAssertIntEquals (test, 0, status);

	return sizeof (data);
}


/*******************
 * Test cases
 *******************/

static void cfm_manager_flash_test_init (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrNotNull (test, manager.base.get_active_cfm);
	CuAssertPtrNotNull (test, manager.base.get_pending_cfm);
	CuAssertPtrNotNull (test, manager.base.free_cfm);
	CuAssertPtrNotNull (test, manager.base.base.activate_pending_manifest);
	CuAssertPtrNotNull (test, manager.base.base.clear_pending_region);
	CuAssertPtrNotNull (test, manager.base.base.write_pending_data);
	CuAssertPtrNotNull (test, manager.base.base.verify_pending_manifest);
	CuAssertPtrNotNull (test, manager.base.base.clear_all_manifests);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	manager.base.get_active_cfm (&manager.base);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_init_only_active_region1 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);

	/* Use blank check to simulate empty CFM regions. */
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_init_only_active_region2 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);

	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, &cfm2, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_init_only_pending_region2 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);

	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, &cfm2, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_init_only_pending_region1 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);

	/* Use blank check to simulate empty CFM regions. */
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, &cfm1, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_init_active_and_pending (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, &cfm2, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_init_region2_pending_lower_id (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_init_region2_pending_same_id (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_init_region1_pending_lower_id (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, &cfm2, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_init_region1_pending_same_id (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, &cfm2, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_init_null (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (NULL, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, MANIFEST_MANAGER_INVALID_ARGUMENT, status);

	status = cfm_manager_flash_init (&manager, NULL, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, MANIFEST_MANAGER_INVALID_ARGUMENT, status);

	status = cfm_manager_flash_init (&manager, &cfm1, NULL, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, MANIFEST_MANAGER_INVALID_ARGUMENT, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, NULL, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, MANIFEST_MANAGER_INVALID_ARGUMENT, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, NULL,
		&verification.base);
	CuAssertIntEquals (test, MANIFEST_MANAGER_INVALID_ARGUMENT, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		NULL);
	CuAssertIntEquals (test, MANIFEST_MANAGER_INVALID_ARGUMENT, status);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_init_region1_flash_error (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_expect_xfer (&flash_mock, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_READ_STATUS_REG);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, FLASH_MASTER_XFER_FAILED, status);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_init_region2_flash_error (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);

	status |= flash_master_mock_expect_xfer (&flash_mock, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_READ_STATUS_REG);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, FLASH_MASTER_XFER_FAILED, status);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_init_cfm_bad_signature (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_SIGNATURE, CFM_SIGNATURE_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000 + CFM_SIGNATURE_OFFSET, 0, -1, CFM_SIGNATURE_LEN));

	status |= flash_master_mock_expect_verify_flash (&flash_mock, 0x10000, CFM_DATA,
		CFM_DATA_LEN - CFM_SIGNATURE_LEN);

	status |= mock_expect (&verification.mock, verification.base.verify_signature, &verification,
		RSA_ENGINE_BAD_SIGNATURE, MOCK_ARG_NOT_NULL, MOCK_ARG (CFM_HASH_LEN), MOCK_ARG_NOT_NULL,
		MOCK_ARG (CFM_SIGNATURE_LEN));

	/* Use blank check to simulate empty CFM regions. */
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_init_cfm_bad_signature_ecc (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_SIGNATURE, CFM_SIGNATURE_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000 + CFM_SIGNATURE_OFFSET, 0, -1, CFM_SIGNATURE_LEN));

	status |= flash_master_mock_expect_verify_flash (&flash_mock, 0x10000, CFM_DATA,
		CFM_DATA_LEN - CFM_SIGNATURE_LEN);

	status |= mock_expect (&verification.mock, verification.base.verify_signature, &verification,
		ECC_ENGINE_BAD_SIGNATURE, MOCK_ARG_NOT_NULL, MOCK_ARG (CFM_HASH_LEN), MOCK_ARG_NOT_NULL,
		MOCK_ARG (CFM_SIGNATURE_LEN));

	/* Use blank check to simulate empty CFM regions. */
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_init_bad_length (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	uint8_t cfm_bad_data[CFM_SIGNATURE_OFFSET];

	TEST_START;

	memcpy (cfm_bad_data, CFM_DATA, sizeof (cfm_bad_data));
	cfm_bad_data[9] = 0xff;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, cfm_bad_data, sizeof (cfm_bad_data),
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));

	/* Use blank check to simulate empty CFM regions. */
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_init_bad_magic_number (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	uint8_t cfm_bad_data[CFM_SIGNATURE_OFFSET];

	TEST_START;

	memcpy (cfm_bad_data, CFM_DATA, sizeof (cfm_bad_data));
	cfm_bad_data[2] ^= 0x55;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, cfm_bad_data, sizeof (cfm_bad_data),
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));

	/* Use blank check to simulate empty CFM regions. */
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_init_region1_id_error (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_xfer (&flash_mock, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_READ_STATUS_REG);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, FLASH_MASTER_XFER_FAILED, status);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_init_region2_id_error (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_xfer (&flash_mock, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_READ_STATUS_REG);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, FLASH_MASTER_XFER_FAILED, status);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_get_active_cfm_null (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);

	/* Use blank check to simulate empty CFM regions. */
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (NULL));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_get_pending_cfm_null (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);

	/* Use blank check to simulate empty CFM regions. */
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (NULL));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_activate_pending_cfm_region2 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	enum manifest_region active;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.activate_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, &cfm2, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	active = state_mgr.get_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM);
	CuAssertIntEquals (test, MANIFEST_REGION_2, active);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_activate_pending_cfm_region1 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	enum manifest_region active;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.activate_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	active = state_mgr.get_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM);
	CuAssertIntEquals (test, MANIFEST_REGION_1, active);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_activate_pending_cfm_region2_notify_observers (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	struct cfm_observer_mock observer;
	int status;
	enum manifest_region active;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_observer_mock_init (&observer);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_add_observer (&manager.base, &observer.base);
	CuAssertIntEquals (test, 0, status);

	status |= mock_expect (&observer.mock, observer.base.on_cfm_activated, &observer, 0,
		MOCK_ARG (&cfm2));

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.activate_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, &cfm2, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	active = state_mgr.get_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM);
	CuAssertIntEquals (test, MANIFEST_REGION_2, active);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = cfm_observer_mock_validate_and_release (&observer);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_activate_pending_cfm_region1_notify_observers (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	struct cfm_observer_mock observer;
	int status;
	enum manifest_region active;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_observer_mock_init (&observer);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_add_observer (&manager.base, &observer.base);
	CuAssertIntEquals (test, 0, status);

	status |= mock_expect (&observer.mock, observer.base.on_cfm_activated, &observer, 0,
		MOCK_ARG (&cfm1));

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.activate_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	active = state_mgr.get_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM);
	CuAssertIntEquals (test, MANIFEST_REGION_1, active);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = cfm_observer_mock_validate_and_release (&observer);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_activate_pending_cfm_no_pending_region2 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);

	/* Use blank check to simulate empty CFM regions. */
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.activate_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, MANIFEST_MANAGER_NONE_PENDING, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_activate_pending_cfm_no_pending_region1 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);

	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.activate_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, MANIFEST_MANAGER_NONE_PENDING, status);

	CuAssertPtrEquals (test, &cfm2, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_activate_pending_cfm_no_pending_notify_observers (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	struct cfm_observer_mock observer;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_observer_mock_init (&observer);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);

	/* Use blank check to simulate empty CFM regions. */
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_add_observer (&manager.base, &observer.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.activate_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, MANIFEST_MANAGER_NONE_PENDING, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = cfm_observer_mock_validate_and_release (&observer);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_activate_pending_cfm_null (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.activate_pending_manifest (NULL);
	CuAssertIntEquals (test, MANIFEST_MANAGER_INVALID_ARGUMENT, status);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_pending_region_region2 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x20000, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_pending_region_region1 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	status |= flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x10000, 0x10000);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_pending_region_invalidate_pending_region2 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	status |= flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x20000, 0x10000);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_pending_region_invalidate_pending_region1 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));

	status |= flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x10000, 0x10000);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, &cfm2, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_pending_region_null (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (NULL, 1);
	CuAssertIntEquals (test, MANIFEST_MANAGER_INVALID_ARGUMENT, status);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_pending_region_manifest_too_large (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, FLASH_BLOCK_SIZE + 1);
	CuAssertIntEquals (test, FLASH_UPDATER_TOO_LARGE, status);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_pending_region_manifest_too_large_with_pending (
	CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, FLASH_BLOCK_SIZE + 1);
	CuAssertIntEquals (test, FLASH_UPDATER_TOO_LARGE, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, &cfm2, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_pending_region_erase_error_region2 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	status |= flash_master_mock_expect_xfer (&flash_mock, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_READ_STATUS_REG);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, FLASH_MASTER_XFER_FAILED, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_pending_region_erase_error_region1 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));

	status |= flash_master_mock_expect_xfer (&flash_mock, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_READ_STATUS_REG);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, FLASH_MASTER_XFER_FAILED, status);

	CuAssertPtrEquals (test, &cfm2, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_pending_region_cfm_in_use_region2 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	struct cfm *pending;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	pending = manager.base.get_pending_cfm (&manager.base);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, MANIFEST_MANAGER_PENDING_IN_USE, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	manager.base.free_cfm (&manager.base, pending);

	status = flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x20000, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	manager.base.get_pending_cfm (&manager.base);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_pending_region_cfm_in_use_region1 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	struct cfm *pending;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	pending = manager.base.get_pending_cfm (&manager.base);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, MANIFEST_MANAGER_PENDING_IN_USE, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	manager.base.free_cfm (&manager.base, pending);

	status = flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x10000, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	manager.base.get_pending_cfm (&manager.base);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_pending_region_cfm_in_use_multiple_region2 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	struct cfm *pending1;
	struct cfm *pending2;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	pending1 = manager.base.get_pending_cfm (&manager.base);
	pending2 = manager.base.get_pending_cfm (&manager.base);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, MANIFEST_MANAGER_PENDING_IN_USE, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	manager.base.free_cfm (&manager.base, pending1);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, MANIFEST_MANAGER_PENDING_IN_USE, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	manager.base.free_cfm (&manager.base, pending2);

	status = flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x20000, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	manager.base.get_pending_cfm (&manager.base);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_pending_region_cfm_in_use_multiple_region1 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	struct cfm *pending1;
	struct cfm *pending2;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	pending1 = manager.base.get_pending_cfm (&manager.base);
	pending2 = manager.base.get_pending_cfm (&manager.base);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, MANIFEST_MANAGER_PENDING_IN_USE, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	manager.base.free_cfm (&manager.base, pending1);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, MANIFEST_MANAGER_PENDING_IN_USE, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	manager.base.free_cfm (&manager.base, pending2);

	status = flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x10000, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	manager.base.get_pending_cfm (&manager.base);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_pending_region_in_use_after_activate_region2 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	struct cfm *active;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	active = manager.base.get_active_cfm (&manager.base);

	status = manager.base.base.activate_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, MANIFEST_MANAGER_PENDING_IN_USE, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	manager.base.free_cfm (&manager.base, active);

	status = flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x10000, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	manager.base.get_pending_cfm (&manager.base);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_pending_region_in_use_after_activate_region1 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	struct cfm *active;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	active = manager.base.get_active_cfm (&manager.base);

	status = manager.base.base.activate_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, MANIFEST_MANAGER_PENDING_IN_USE, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	manager.base.free_cfm (&manager.base, active);

	status = flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x20000, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	manager.base.get_pending_cfm (&manager.base);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_pending_no_pending_in_use_region2 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);

	/* Use blank check to simulate empty CFM regions. */
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	manager.base.get_pending_cfm (&manager.base);

	status = flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x20000, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_pending_no_pending_in_use_region1 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);

	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	manager.base.get_pending_cfm (&manager.base);

	status = flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x10000, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_pending_region_extra_free_call (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	struct cfm *pending;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	pending = manager.base.get_pending_cfm (&manager.base);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, MANIFEST_MANAGER_PENDING_IN_USE, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	manager.base.free_cfm (&manager.base, pending);
	manager.base.free_cfm (&manager.base, pending);

	pending = manager.base.get_pending_cfm (&manager.base);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, MANIFEST_MANAGER_PENDING_IN_USE, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	manager.base.free_cfm (&manager.base, pending);

	status = flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x20000, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	manager.base.get_pending_cfm (&manager.base);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_pending_region_free_null_region2 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	manager.base.get_pending_cfm (&manager.base);
	manager.base.free_cfm (&manager.base, NULL);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, MANIFEST_MANAGER_PENDING_IN_USE, status);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_pending_region_free_null_region1 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	manager.base.get_pending_cfm (&manager.base);
	manager.base.free_cfm (&manager.base, NULL);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, MANIFEST_MANAGER_PENDING_IN_USE, status);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_pending_region_free_null_manager (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	struct cfm *pending;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	pending = manager.base.get_pending_cfm (&manager.base);
	manager.base.free_cfm (NULL, pending);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, MANIFEST_MANAGER_PENDING_IN_USE, status);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_write_pending_data_region2 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	uint8_t data[] = {0x01, 0x02, 0x03, 0x04};

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	status |= flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x20000, 0x10000);
	status |= flash_master_mock_expect_write (&flash_mock, 0x20000, data, sizeof (data));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.write_pending_data (&manager.base.base, data, sizeof (data));
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_write_pending_data_region1 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	uint8_t data[] = {0x01, 0x02, 0x03, 0x04};

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	status |= flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x10000, 0x10000);
	status |= flash_master_mock_expect_write (&flash_mock, 0x10000, data, sizeof (data));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.write_pending_data (&manager.base.base, data, sizeof (data));
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_write_pending_data_multiple (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	uint8_t data1[] = {0x01, 0x02, 0x03, 0x04};
	uint8_t data2[] = {0x05, 0x06, 0x07, 0x08, 0x09};
	uint8_t data3[] = {0x0a, 0x0b, 0x0c};

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	status |= flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x20000, 0x10000);

	status |= flash_master_mock_expect_write (&flash_mock, 0x20000, data1, 4);
	status |= flash_master_mock_expect_write (&flash_mock, 0x20004, data2, 5);
	status |= flash_master_mock_expect_write (&flash_mock, 0x20009, data3, 3);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.write_pending_data (&manager.base.base, data1, sizeof (data1));
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.write_pending_data (&manager.base.base, data2, sizeof (data2));
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.write_pending_data (&manager.base.base, data3, sizeof (data3));
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_write_pending_data_block_end (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
	uint8_t fill[FLASH_BLOCK_SIZE - sizeof (data)] = {0};

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	status |= flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x20000, 0x10000);
	status |= flash_master_mock_expect_write (&flash_mock, 0x20000, fill, sizeof (fill));
	status |= flash_master_mock_expect_write (&flash_mock, 0x2fffc, data, sizeof (data));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	/* Fill with data to write at the end of the flash block. */
	status = manager.base.base.write_pending_data (&manager.base.base, fill, sizeof (fill));
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.write_pending_data (&manager.base.base, data, sizeof (data));
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_write_pending_data_null (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	uint8_t data[] = {0x01, 0x02, 0x03, 0x04};

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	status |= flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x20000, 0x10000);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.write_pending_data (NULL, data, sizeof (data));
	CuAssertIntEquals (test, MANIFEST_MANAGER_INVALID_ARGUMENT, status);

	status = manager.base.base.write_pending_data (&manager.base.base, NULL, sizeof (data));
	CuAssertIntEquals (test, MANIFEST_MANAGER_INVALID_ARGUMENT, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_write_pending_data_write_error (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	uint8_t data[] = {0x01, 0x02, 0x03, 0x04};

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	status |= flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x20000, 0x10000);
	status |= flash_master_mock_expect_xfer (&flash_mock, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_READ_STATUS_REG);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.write_pending_data (&manager.base.base, data, sizeof (data));
	CuAssertIntEquals (test, FLASH_MASTER_XFER_FAILED, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_write_pending_data_write_after_error (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	uint8_t data1[] = {0x01, 0x02, 0x03, 0x04};
	uint8_t data2[] = {0x05, 0x06, 0x07, 0x08, 0x09};
	uint8_t data3[] = {0x0a, 0x0b, 0x0c};

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	status |= flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x20000, 0x10000);

	status |= flash_master_mock_expect_write (&flash_mock, 0x20000, data1, 4);
	status |= flash_master_mock_expect_xfer (&flash_mock, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_write (&flash_mock, 0x20004, data3, 3);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.write_pending_data (&manager.base.base, data1, sizeof (data1));
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.write_pending_data (&manager.base.base, data2, sizeof (data2));
	CuAssertIntEquals (test, FLASH_MASTER_XFER_FAILED, status);

	status = manager.base.base.write_pending_data (&manager.base.base, data3, sizeof (data3));
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_write_pending_data_partial_write (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
	uint8_t fill[FLASH_PAGE_SIZE - 1] = {0};

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	status |= flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x20000, 0x10000);
	status |= flash_master_mock_expect_write (&flash_mock, 0x20000, fill, sizeof (fill));

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_xfer (&flash_mock, 0, FLASH_EXP_WRITE_ENABLE);
	status |= flash_master_mock_expect_tx_xfer (&flash_mock, 0,
		FLASH_EXP_WRITE_CMD (0x02, 0x200ff, 0, data, 1));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);

	status |= flash_master_mock_expect_xfer (&flash_mock, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_WRITE_ENABLE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	/* Partially fill the page to force a write across pages. */
	status = manager.base.base.write_pending_data (&manager.base.base, fill, sizeof (fill));
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.write_pending_data (&manager.base.base, data, sizeof (data));
	CuAssertIntEquals (test, FLASH_UPDATER_INCOMPLETE_WRITE, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_write_pending_data_write_after_partial_write (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	uint8_t data1[] = {0x01, 0x02, 0x03, 0x04};
	uint8_t data2[] = {0x05, 0x06, 0x07, 0x08, 0x09};
	uint8_t fill[FLASH_PAGE_SIZE - 1] = {0};

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	status |= flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x20000, 0x10000);
	status |= flash_master_mock_expect_write (&flash_mock, 0x20000, fill, sizeof (fill));

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_xfer (&flash_mock, 0, FLASH_EXP_WRITE_ENABLE);
	status |= flash_master_mock_expect_tx_xfer (&flash_mock, 0,
		FLASH_EXP_WRITE_CMD (0x02, 0x200ff, 0, data1, 1));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);

	status |= flash_master_mock_expect_xfer (&flash_mock, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_WRITE_ENABLE);

	status |= flash_master_mock_expect_write (&flash_mock, 0x20100, data2, 5);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	/* Partially fill the page to force a write across pages. */
	status = manager.base.base.write_pending_data (&manager.base.base, fill, sizeof (fill));
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.write_pending_data (&manager.base.base, data1, sizeof (data1));
	CuAssertIntEquals (test, FLASH_UPDATER_INCOMPLETE_WRITE, status);

	status = manager.base.base.write_pending_data (&manager.base.base, data2, sizeof (data2));
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_write_pending_data_without_clear (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	uint8_t data[] = {0x01, 0x02, 0x03, 0x04};

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.write_pending_data (&manager.base.base, data, sizeof (data));
	CuAssertIntEquals (test, MANIFEST_MANAGER_NOT_CLEARED, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_write_pending_data_restart_write (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	uint8_t data1[] = {0x01, 0x02, 0x03, 0x04};
	uint8_t data2[] = {0x05, 0x06, 0x07, 0x08, 0x09};
	uint8_t data3[] = {0x0a, 0x0b, 0x0c};

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	status |= flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x20000, 0x10000);

	status |= flash_master_mock_expect_write (&flash_mock, 0x20000, data1, 4);
	status |= flash_master_mock_expect_write (&flash_mock, 0x20004, data2, 5);

	status |= flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x20000, 0x10000);

	status |= flash_master_mock_expect_write (&flash_mock, 0x20000, data3, 3);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.write_pending_data (&manager.base.base, data1, sizeof (data1));
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.write_pending_data (&manager.base.base, data2, sizeof (data2));
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.write_pending_data (&manager.base.base, data3, sizeof (data3));
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_write_pending_data_too_long (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
	uint8_t fill[FLASH_BLOCK_SIZE - sizeof (data) + 1] = {0};

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	status |= flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x20000, 0x10000);
	status |= flash_master_mock_expect_write (&flash_mock, 0x20000, fill, sizeof (fill));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	/* Fill with data to write at the end of the flash block. */
	status = manager.base.base.write_pending_data (&manager.base.base, fill, sizeof (fill));
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.write_pending_data (&manager.base.base, data, sizeof (data));
	CuAssertIntEquals (test, FLASH_UPDATER_OUT_OF_SPACE, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_write_pending_data_cfm_in_use (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	struct cfm *pending;
	uint8_t data[] = {0x01, 0x02, 0x03, 0x04};

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	pending = manager.base.get_pending_cfm (&manager.base);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, MANIFEST_MANAGER_PENDING_IN_USE, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.write_pending_data (&manager.base.base, data, sizeof (data));
	CuAssertIntEquals (test, MANIFEST_MANAGER_NOT_CLEARED, status);

	manager.base.free_cfm (&manager.base, pending);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_region2 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x20000);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, &cfm2, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_region1 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x10000);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, &cfm1, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_region2_notify_observers (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	struct cfm_observer_mock observer;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_observer_mock_init (&observer);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x20000);

	status = cfm_manager_add_observer (&manager.base, &observer.base);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);
	status |= mock_expect (&observer.mock, observer.base.on_cfm_verified, &observer, 0,
		MOCK_ARG (&cfm2));

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, &cfm2, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = cfm_observer_mock_validate_and_release (&observer);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_region1_notify_observers (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	struct cfm_observer_mock observer;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_observer_mock_init (&observer);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x10000);

	status = cfm_manager_add_observer (&manager.base, &observer.base);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= mock_expect (&observer.mock, observer.base.on_cfm_verified, &observer, 0,
		MOCK_ARG (&cfm1));

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, &cfm1, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = cfm_observer_mock_validate_and_release (&observer);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_already_valid_region2 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);

	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, &cfm2, manager.base.get_pending_cfm (&manager.base));

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, MANIFEST_MANAGER_HAS_PENDING, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, &cfm2, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_already_valid_region1 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);

	/* Use blank check to simulate empty CFM regions. */
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, &cfm1, manager.base.get_pending_cfm (&manager.base));

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, MANIFEST_MANAGER_HAS_PENDING, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, &cfm1, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_already_valid_notify_observers (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	struct cfm_observer_mock observer;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_observer_mock_init (&observer);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);

	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, &cfm2, manager.base.get_pending_cfm (&manager.base));

	status = cfm_manager_add_observer (&manager.base, &observer.base);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, MANIFEST_MANAGER_HAS_PENDING, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, &cfm2, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = cfm_observer_mock_validate_and_release (&observer);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_with_active (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);

	/* Use blank check to simulate empty CFM regions. */
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x20000);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, &cfm2, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_already_valid_with_active (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, &cfm2, manager.base.get_pending_cfm (&manager.base));

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, MANIFEST_MANAGER_HAS_PENDING, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, &cfm2, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_lower_id (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x10000);

	/* Use blank check to simulate empty CFM regions. */
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x20000);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, MANIFEST_MANAGER_INVALID_ID, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_same_id (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);

	/* Use blank check to simulate empty CFM regions. */
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x20000);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, MANIFEST_MANAGER_INVALID_ID, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_no_clear_region2 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, MANIFEST_MANAGER_NONE_PENDING, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_no_clear_region1 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, MANIFEST_MANAGER_NONE_PENDING, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_extra_data_written (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
	int offset;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	offset = cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x20000);

	status = flash_master_mock_expect_write (&flash_mock, 0x20000 + offset, data, sizeof (data));

	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.write_pending_data (&manager.base.base, data, sizeof (data));
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, &cfm2, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_null (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x20000);

	status = manager.base.base.verify_pending_manifest (NULL);
	CuAssertIntEquals (test, MANIFEST_MANAGER_INVALID_ARGUMENT, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_verify_error_region2 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x20000);

	status = flash_master_mock_expect_xfer (&flash_mock, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_READ_STATUS_REG);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, FLASH_MASTER_XFER_FAILED, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_verify_error_region1 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x10000);

	status = flash_master_mock_expect_xfer (&flash_mock, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_READ_STATUS_REG);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, FLASH_MASTER_XFER_FAILED, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_verify_error_notify_observers (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	struct cfm_observer_mock observer;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_observer_mock_init (&observer);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x20000);

	status = cfm_manager_add_observer (&manager.base, &observer.base);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_expect_xfer (&flash_mock, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_READ_STATUS_REG);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, FLASH_MASTER_XFER_FAILED, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = cfm_observer_mock_validate_and_release (&observer);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_verify_fail_region2 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x20000);

	status = flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_SIGNATURE, CFM2_SIGNATURE_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000 + CFM_SIGNATURE_OFFSET, 0, -1, CFM2_SIGNATURE_LEN));

	status |= flash_master_mock_expect_verify_flash (&flash_mock, 0x20000, CFM_DATA,
		CFM_DATA_LEN - CFM_SIGNATURE_LEN);

	status |= mock_expect (&verification.mock, verification.base.verify_signature, &verification,
		RSA_ENGINE_BAD_SIGNATURE, MOCK_ARG_NOT_NULL, MOCK_ARG (CFM_HASH_LEN), MOCK_ARG_NOT_NULL,
		MOCK_ARG (CFM_SIGNATURE_LEN));

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, RSA_ENGINE_BAD_SIGNATURE, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_verify_fail_region1 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x10000);

	status = flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_SIGNATURE, CFM_SIGNATURE_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000 + CFM_SIGNATURE_OFFSET, 0, -1, CFM_SIGNATURE_LEN));

	status |= flash_master_mock_expect_verify_flash (&flash_mock, 0x10000, CFM_DATA,
		CFM_DATA_LEN - CFM_SIGNATURE_LEN);

	status |= mock_expect (&verification.mock, verification.base.verify_signature, &verification,
		RSA_ENGINE_BAD_SIGNATURE, MOCK_ARG_NOT_NULL, MOCK_ARG (CFM_HASH_LEN), MOCK_ARG_NOT_NULL,
		MOCK_ARG (CFM_SIGNATURE_LEN));

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, RSA_ENGINE_BAD_SIGNATURE, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_verify_fail_ecc_region2 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x20000);

	status = flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_SIGNATURE, CFM2_SIGNATURE_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000 + CFM_SIGNATURE_OFFSET, 0, -1, CFM2_SIGNATURE_LEN));

	status |= flash_master_mock_expect_verify_flash (&flash_mock, 0x20000, CFM_DATA,
		CFM_DATA_LEN - CFM_SIGNATURE_LEN);

	status |= mock_expect (&verification.mock, verification.base.verify_signature, &verification,
		ECC_ENGINE_BAD_SIGNATURE, MOCK_ARG_NOT_NULL, MOCK_ARG (CFM_HASH_LEN), MOCK_ARG_NOT_NULL,
		MOCK_ARG (CFM_SIGNATURE_LEN));

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, ECC_ENGINE_BAD_SIGNATURE, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_verify_fail_ecc_region1 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x10000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_SIGNATURE, CFM_SIGNATURE_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000 + CFM_SIGNATURE_OFFSET, 0, -1, CFM_SIGNATURE_LEN));

	status |= flash_master_mock_expect_verify_flash (&flash_mock, 0x10000, CFM_DATA,
		CFM_DATA_LEN - CFM_SIGNATURE_LEN);

	status |= mock_expect (&verification.mock, verification.base.verify_signature, &verification,
		ECC_ENGINE_BAD_SIGNATURE, MOCK_ARG_NOT_NULL, MOCK_ARG (CFM_HASH_LEN), MOCK_ARG_NOT_NULL,
		MOCK_ARG (CFM_SIGNATURE_LEN));

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, ECC_ENGINE_BAD_SIGNATURE, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_verify_after_verify_error (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x20000);

	status = flash_master_mock_expect_xfer (&flash_mock, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_READ_STATUS_REG);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, FLASH_MASTER_XFER_FAILED, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, MANIFEST_MANAGER_NONE_PENDING, status);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_verify_after_verify_fail (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x20000);

	status = flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_SIGNATURE, CFM2_SIGNATURE_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000 + CFM_SIGNATURE_OFFSET, 0, -1, CFM2_SIGNATURE_LEN));

	status |= flash_master_mock_expect_verify_flash (&flash_mock, 0x20000, CFM_DATA,
		CFM_DATA_LEN - CFM_SIGNATURE_LEN);

	status |= mock_expect (&verification.mock, verification.base.verify_signature, &verification,
		RSA_ENGINE_BAD_SIGNATURE, MOCK_ARG_NOT_NULL, MOCK_ARG (CFM_HASH_LEN), MOCK_ARG_NOT_NULL,
		MOCK_ARG (CFM_SIGNATURE_LEN));

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, RSA_ENGINE_BAD_SIGNATURE, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, MANIFEST_MANAGER_NONE_PENDING, status);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_write_after_verify (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	uint8_t data[] = {0x01, 0x02, 0x03, 0x04};

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x20000);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, &cfm2, manager.base.get_pending_cfm (&manager.base));

	status = manager.base.base.write_pending_data (&manager.base.base, data, sizeof (data));
	CuAssertIntEquals (test, MANIFEST_MANAGER_NOT_CLEARED, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, &cfm2, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_write_after_verify_error (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	uint8_t data[] = {0x01, 0x02, 0x03, 0x04};

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x20000);

	status = flash_master_mock_expect_xfer (&flash_mock, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_READ_STATUS_REG);

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, FLASH_MASTER_XFER_FAILED, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = manager.base.base.write_pending_data (&manager.base.base, data, sizeof (data));
	CuAssertIntEquals (test, MANIFEST_MANAGER_NOT_CLEARED, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_with_active_id2_error_region2 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);

	/* Use blank check to simulate empty CFM regions. */
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x20000);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_xfer (&flash_mock, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_READ_STATUS_REG);

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, FLASH_MASTER_XFER_FAILED, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_with_active_id2_error_region1 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);

	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, &cfm2, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x10000);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x10000);

	status |= flash_master_mock_expect_xfer (&flash_mock, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_READ_STATUS_REG);

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, FLASH_MASTER_XFER_FAILED, status);

	CuAssertPtrEquals (test, &cfm2, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_with_active_id1_error (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);

	/* Use blank check to simulate empty CFM regions. */
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x20000);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_xfer (&flash_mock, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_READ_STATUS_REG);

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, FLASH_MASTER_XFER_FAILED, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_write_after_id_error (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	uint8_t data[] = {0x01, 0x02, 0x03, 0x04};

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);

	/* Use blank check to simulate empty CFM regions. */
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	cfm_manager_flash_testing_write_new_cfm (test, &manager, &flash_mock, 0x20000);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_xfer (&flash_mock, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_READ_STATUS_REG);

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, FLASH_MASTER_XFER_FAILED, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = manager.base.base.write_pending_data (&manager.base.base, data, sizeof (data));
	CuAssertIntEquals (test, MANIFEST_MANAGER_NOT_CLEARED, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_incomplete_cfm (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x20000, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 2);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, MANIFEST_MANAGER_INCOMPLETE_UPDATE, status);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_verify_pending_cfm_write_after_incomplete_cfm (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	uint8_t data[] = {0x01, 0x02, 0x03, 0x04};

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x20000, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 2);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.verify_pending_manifest (&manager.base.base);
	CuAssertIntEquals (test, MANIFEST_MANAGER_INCOMPLETE_UPDATE, status);

	status = manager.base.base.write_pending_data (&manager.base.base, data, sizeof (data));
	CuAssertIntEquals (test, MANIFEST_MANAGER_NOT_CLEARED, status);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_all_manifests_region1 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_expect_erase_flash (&flash_mock, 0x20000);
	status |= flash_master_mock_expect_erase_flash (&flash_mock, 0x10000);

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_all_manifests (&manager.base.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_all_manifests_region2 (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = state_mgr.save_active_manifest (&state_mgr, SYSTEM_STATE_MANIFEST_CFM,
		MANIFEST_REGION_2);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_expect_erase_flash (&flash_mock, 0x10000);
	status |= flash_master_mock_expect_erase_flash (&flash_mock, 0x20000);

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_all_manifests (&manager.base.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_all_manifests_only_active (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);

	/* Use blank check to simulate empty CFM regions. */
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_expect_erase_flash (&flash_mock, 0x20000);
	status |= flash_master_mock_expect_erase_flash (&flash_mock, 0x10000);

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_all_manifests (&manager.base.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_all_manifests_only_pending (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);

	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x20000);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_expect_erase_flash (&flash_mock, 0x20000);
	status |= flash_master_mock_expect_erase_flash (&flash_mock, 0x10000);

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_all_manifests (&manager.base.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_all_manifests_no_cfms (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	/* Use blank check to simulate empty CFM regions. */
	status = flash_master_mock_expect_blank_check (&flash_mock, 0x10000, CFM_HEADER_SIZE);
	status |= flash_master_mock_expect_blank_check (&flash_mock, 0x20000, CFM_HEADER_SIZE);

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_expect_erase_flash (&flash_mock, 0x20000);
	status |= flash_master_mock_expect_erase_flash (&flash_mock, 0x10000);

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_all_manifests (&manager.base.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_all_manifests_pending_in_use (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	struct cfm *pending;
	struct cfm *active;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	pending = manager.base.get_pending_cfm (&manager.base);

	status = manager.base.base.clear_all_manifests (&manager.base.base);
	CuAssertIntEquals (test, MANIFEST_MANAGER_PENDING_IN_USE, status);

	manager.base.free_cfm (&manager.base, pending);

	active = manager.base.get_active_cfm (&manager.base);
	CuAssertPtrEquals (test, &cfm1, active);
	manager.base.free_cfm (&manager.base, active);

	pending = manager.base.get_pending_cfm (&manager.base);
	CuAssertPtrEquals (test, &cfm2, pending);
	manager.base.free_cfm (&manager.base, pending);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_expect_erase_flash (&flash_mock, 0x20000);
	status |= flash_master_mock_expect_erase_flash (&flash_mock, 0x10000);

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_all_manifests (&manager.base.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_all_manifests_active_in_use (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	struct cfm *pending;
	struct cfm *active;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	active = manager.base.get_active_cfm (&manager.base);

	status = flash_master_mock_expect_erase_flash (&flash_mock, 0x20000);

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_all_manifests (&manager.base.base);
	CuAssertIntEquals (test, MANIFEST_MANAGER_ACTIVE_IN_USE, status);

	manager.base.free_cfm (&manager.base, active);

	active = manager.base.get_active_cfm (&manager.base);
	CuAssertPtrEquals (test, &cfm1, active);
	manager.base.free_cfm (&manager.base, active);

	pending = manager.base.get_pending_cfm (&manager.base);
	CuAssertPtrEquals (test, NULL, pending);
	manager.base.free_cfm (&manager.base, pending);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_expect_erase_flash (&flash_mock, 0x20000);
	status |= flash_master_mock_expect_erase_flash (&flash_mock, 0x10000);

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_all_manifests (&manager.base.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_all_manifests_during_update (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;
	uint8_t data[] = {0x01, 0x02, 0x03, 0x04};

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_expect_erase_flash_verify (&flash_mock, 0x20000, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_pending_region (&manager.base.base, 1);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_expect_erase_flash (&flash_mock, 0x20000);
	status |= flash_master_mock_expect_erase_flash (&flash_mock, 0x10000);

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_all_manifests (&manager.base.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = manager.base.base.write_pending_data (&manager.base.base, data, sizeof (data));
	CuAssertIntEquals (test, MANIFEST_MANAGER_NOT_CLEARED, status);

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_all_manifests_null (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_all_manifests (NULL);
	CuAssertIntEquals (test, MANIFEST_MANAGER_INVALID_ARGUMENT, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, &cfm2, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_all_manifests_erase_pending_error (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_expect_xfer (&flash_mock, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_READ_STATUS_REG);

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_all_manifests (&manager.base.base);
	CuAssertIntEquals (test, FLASH_MASTER_XFER_FAILED, status);

	CuAssertPtrEquals (test, &cfm1, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}

static void cfm_manager_flash_test_clear_all_manifests_erase_active_error (CuTest *test)
{
	HASH_TESTING_ENGINE hash;
	struct signature_verification_mock verification;
	struct flash_master_mock flash_mock;
	struct flash_master_mock flash_mock_state;
	struct spi_flash flash;
	struct spi_flash flash_state;
	struct state_manager state_mgr;
	struct cfm_flash cfm1;
	struct cfm_flash cfm2;
	struct cfm_manager_flash manager;
	int status;

	TEST_START;

	status = HASH_TESTING_ENGINE_INIT (&hash);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_init (&verification);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_init (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash, &flash_mock.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash, 0x1000000);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_testing_init_system_state (test, &state_mgr, &flash_mock_state, &flash_state);

	status = cfm_flash_init (&cfm1, &flash, 0x10000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_flash_init (&cfm2, &flash, 0x20000);
	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM_DATA,
		CFM_DATA_LEN, CFM_HASH, CFM_SIGNATURE, CFM_SIGNATURE_OFFSET, 0x10000);
	status |= cfm_manager_flash_testing_verify_cfm (&flash_mock, &verification, CFM2_DATA,
		CFM2_DATA_LEN, CFM2_HASH, CFM2_SIGNATURE, CFM2_SIGNATURE_OFFSET, 0x20000);

	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM_DATA, CFM_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x10000, 0, -1, CFM_HEADER_SIZE));
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, &WIP_STATUS, 1,
		FLASH_EXP_READ_STATUS_REG);
	status |= flash_master_mock_expect_rx_xfer (&flash_mock, 0, CFM2_DATA, CFM2_DATA_LEN,
		FLASH_EXP_READ_CMD (0x03, 0x20000, 0, -1, CFM_HEADER_SIZE));

	CuAssertIntEquals (test, 0, status);

	status = cfm_manager_flash_init (&manager, &cfm1, &cfm2, &state_mgr, &hash.base,
		&verification.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash_mock.mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_expect_erase_flash (&flash_mock, 0x20000);
	status |= flash_master_mock_expect_xfer (&flash_mock, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_READ_STATUS_REG);

	CuAssertIntEquals (test, 0, status);

	status = manager.base.base.clear_all_manifests (&manager.base.base);
	CuAssertIntEquals (test, FLASH_MASTER_XFER_FAILED, status);

	CuAssertPtrEquals (test, NULL, manager.base.get_active_cfm (&manager.base));
	CuAssertPtrEquals (test, NULL, manager.base.get_pending_cfm (&manager.base));

	status = flash_master_mock_validate_and_release (&flash_mock);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash_mock_state);
	CuAssertIntEquals (test, 0, status);

	status = signature_verification_mock_validate_and_release (&verification);
	CuAssertIntEquals (test, 0, status);

	cfm_manager_flash_release (&manager);

	system_state_manager_release (&state_mgr);
	cfm_flash_release (&cfm1);
	cfm_flash_release (&cfm2);
	spi_flash_release (&flash);
	spi_flash_release (&flash_state);
	HASH_TESTING_ENGINE_RELEASE (&hash);
}


CuSuite* get_cfm_manager_flash_suite ()
{
	CuSuite *suite = CuSuiteNew ();

	SUITE_ADD_TEST (suite, cfm_manager_flash_test_init);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_init_only_active_region1);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_init_only_active_region2);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_init_only_pending_region2);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_init_only_pending_region1);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_init_active_and_pending);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_init_region2_pending_lower_id);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_init_region2_pending_same_id);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_init_region1_pending_lower_id);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_init_region1_pending_same_id);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_init_null);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_init_region1_flash_error);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_init_region2_flash_error);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_init_cfm_bad_signature);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_init_cfm_bad_signature_ecc);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_init_bad_length);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_init_bad_magic_number);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_init_region1_id_error);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_init_region2_id_error);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_get_active_cfm_null);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_get_pending_cfm_null);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_activate_pending_cfm_region2);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_activate_pending_cfm_region1);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_activate_pending_cfm_region2_notify_observers);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_activate_pending_cfm_region1_notify_observers);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_activate_pending_cfm_no_pending_region2);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_activate_pending_cfm_no_pending_region1);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_activate_pending_cfm_no_pending_notify_observers);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_activate_pending_cfm_null);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_pending_region_region2);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_pending_region_region1);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_pending_region_invalidate_pending_region2);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_pending_region_invalidate_pending_region1);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_pending_region_null);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_pending_region_manifest_too_large);
	SUITE_ADD_TEST (suite,
		cfm_manager_flash_test_clear_pending_region_manifest_too_large_with_pending);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_pending_region_erase_error_region2);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_pending_region_erase_error_region1);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_pending_region_cfm_in_use_region2);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_pending_region_cfm_in_use_region1);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_pending_region_cfm_in_use_multiple_region2);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_pending_region_cfm_in_use_multiple_region1);
	SUITE_ADD_TEST (suite,
		cfm_manager_flash_test_clear_pending_region_in_use_after_activate_region2);
	SUITE_ADD_TEST (suite,
		cfm_manager_flash_test_clear_pending_region_in_use_after_activate_region1);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_pending_no_pending_in_use_region2);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_pending_no_pending_in_use_region1);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_pending_region_extra_free_call);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_pending_region_free_null_region2);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_pending_region_free_null_region1);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_pending_region_free_null_manager);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_write_pending_data_region2);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_write_pending_data_region1);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_write_pending_data_multiple);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_write_pending_data_block_end);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_write_pending_data_null);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_write_pending_data_write_error);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_write_pending_data_write_after_error);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_write_pending_data_partial_write);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_write_pending_data_write_after_partial_write);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_write_pending_data_without_clear);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_write_pending_data_restart_write);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_write_pending_data_too_long);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_write_pending_data_cfm_in_use);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_region2);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_region1);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_region2_notify_observers);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_region1_notify_observers);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_already_valid_region2);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_already_valid_region1);
	SUITE_ADD_TEST (suite,
		cfm_manager_flash_test_verify_pending_cfm_already_valid_notify_observers);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_with_active);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_already_valid_with_active);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_lower_id);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_same_id);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_no_clear_region2);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_no_clear_region1);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_extra_data_written);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_null);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_verify_error_region2);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_verify_error_region1);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_verify_error_notify_observers);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_verify_fail_region2);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_verify_fail_region1);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_verify_fail_ecc_region2);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_verify_fail_ecc_region1);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_verify_after_verify_error);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_verify_after_verify_fail);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_write_after_verify);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_write_after_verify_error);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_with_active_id2_error_region2);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_with_active_id2_error_region1);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_with_active_id1_error);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_write_after_id_error);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_incomplete_cfm);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_verify_pending_cfm_write_after_incomplete_cfm);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_all_manifests_region1);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_all_manifests_region2);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_all_manifests_only_active);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_all_manifests_only_pending);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_all_manifests_no_cfms);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_all_manifests_pending_in_use);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_all_manifests_active_in_use);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_all_manifests_during_update);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_all_manifests_null);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_all_manifests_erase_pending_error);
	SUITE_ADD_TEST (suite, cfm_manager_flash_test_clear_all_manifests_erase_active_error);

	return suite;
}
