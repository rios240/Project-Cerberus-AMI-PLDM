// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#ifndef HOST_IRQ_HANDLER_H_
#define HOST_IRQ_HANDLER_H_

#include <stdbool.h>
#include "host_processor.h"
#include "bmc_recovery.h"
#include "crypto/hash.h"
#include "crypto/rsa.h"


/**
 * Handler instance for processing interrupts from the host processor.
 */
struct host_irq_handler {
	/**
	 * Handler for initial host processing at power on.
	 *
	 * @param handler The handler context.
	 * @param allow_unsecure Flag indicating if the host should be allowed to boot regardless of the
	 * authentication status.
	 *
	 * @return 0 if power on processing was successful or an error code.
	 */
	int (*power_on) (struct host_irq_handler *handler, bool allow_unsecure);

	/**
	 * Handler for when the host processor signals that it has been reset.
	 *
	 * @param handler The handler context.
	 *
	 * @return 0 if host reset processing was successful or an error code.
	 */
	int (*enter_reset) (struct host_irq_handler *handler);

	/**
	 * Handler for when the host processor has left the reset state and is booting.
	 *
	 * @param handler The handler context.
	 */
	void (*exit_reset) (struct host_irq_handler *handler);

	/**
	 * Handler for when the host processor asserts SPI chip select 0.
	 *
	 * @param handler The handler context.
	 */
	void (*assert_cs0) (struct host_irq_handler *handler);

	/**
	 * Handler for when the host processor asserts SPI chip select 1.
	 *
	 * @param handler The handler context.
	 *
	 * @return 0 if flash recovery was successful or not attempted.  Otherwise, the recovery error
	 * code.
	 */
	int (*assert_cs1) (struct host_irq_handler *handler);

	struct host_processor *host;			/**< The host generating the IRQs. */
	struct bmc_recovery *recovery;			/**< The recovery manager for BMC watchdog failover. */
	struct hash_engine *hash;				/**< Hash engine to use for reset validation. */
	struct rsa_engine *rsa;					/**< RSA engine to use for reset validation. */
};


int host_irq_handler_init (struct host_irq_handler *handler, struct host_processor *host,
	struct hash_engine *hash, struct rsa_engine *rsa, struct bmc_recovery *recovery);
void host_irq_handler_release (struct host_irq_handler *handler);

int host_irq_handler_set_host (struct host_irq_handler *handler, struct host_processor *host);

/* Internal functions for use by derived types. */
int host_irq_handler_power_on (struct host_irq_handler *handler, bool allow_unsecure);
int host_irq_handler_enter_reset (struct host_irq_handler *handler);
void host_irq_handler_exit_reset (struct host_irq_handler *handler);
void host_irq_handler_assert_cs0 (struct host_irq_handler *handler);
int host_irq_handler_assert_cs1 (struct host_irq_handler *handler);


#define	HOST_IRQ_HANDLER_ERROR(code)		ROT_ERROR (ROT_MODULE_HOST_IRQ_HANDLER, code)

/**
 * Error codes that can be generated by a host IRQ handler.
 */
enum {
	HOST_IRQ_HANDLER_INVALID_ARGUMENT = HOST_IRQ_HANDLER_ERROR (0x00),		/**< Input parameter is null or not valid. */
	HOST_IRQ_HANDLER_NO_MEMORY = HOST_IRQ_HANDLER_ERROR (0x01),				/**< Memory allocation failed. */
	HOST_IRQ_HANDLER_ENTER_RESET_FAILED = HOST_IRQ_HANDLER_ERROR (0x02),	/**< Failure while processing an enter reset event. */
	HOST_IRQ_HANDLER_EXIT_RESET_FAILED = HOST_IRQ_HANDLER_ERROR (0x03),		/**< Failure while processing an exit reset event. */
	HOST_IRQ_HANDLER_CS0_FAILED = HOST_IRQ_HANDLER_ERROR (0x04),			/**< Failure while processing a CS0 reset event. */
	HOST_IRQ_HANDLER_CS1_FAILED = HOST_IRQ_HANDLER_ERROR (0x05),			/**< Failure while processing a CS1 reset event. */
};


#endif /* HOST_IRQ_HANDLER_H_ */
