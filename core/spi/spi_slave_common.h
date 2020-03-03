// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#ifndef SPI_SLAVE_COMMON_H_
#define SPI_SLAVE_COMMON_H_

#include <stdint.h>
#include <stdbool.h>
#include "status/rot_status.h"


#define	SPI_SLAVE_ERROR(code)		ROT_ERROR (ROT_MODULE_SPI_SLAVE, code)


/**
 * Error codes that can be generated by a SPI slave driver.
 */
enum {
	SPI_SLAVE_INVALID_ARGUMENT = SPI_SLAVE_ERROR (0),		/**< Input parameter is null or not valid. */
	SPI_SLAVE_NO_MEMORY = SPI_SLAVE_ERROR (1),				/**< Memory allocation failed. */
	SPI_SLAVE_BUSY = SPI_SLAVE_ERROR (2),					/**< SPI slave busy. */
	SPI_SLAVE_TX_FAILED = SPI_SLAVE_ERROR (3),				/**< SPI slave failed to write to master. */
};


#endif /* SPI_SLAVE_COMMON_H_ */
