// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#ifndef CHECKSUM_H_
#define CHECKSUM_H_

#include <stdint.h>


uint8_t checksum_crc8 (uint8_t smbus_addr, uint8_t *data, uint8_t len);


#endif //CHECKSUM_H_
