// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#ifndef CMD_LOGGING_H_
#define CMD_LOGGING_H_

#include "logging/debug_log.h"


/**
 * Logging messages for command handling.
 *
 * Note: Commented types are deprecated
 */
enum {
	CMD_LOGGING_PROCESS_FAIL,				/**< Error while processing a received command. */
	CMD_LOGGING_PACKET_OVERFLOW,			/**< A received message exceeded the maximum length. */
//	CMD_LOGGING_PROTOCOL_ERROR,				/**< Error while processing input in MCTP protocol layer. */
	CMD_LOGGING_SEND_PACKET_FAIL = 3,		/**< Error sending a packet over a command channel. */
	CMD_LOGGING_RECEIVE_PACKET_FAIL,		/**< Error receiving a packet over a command channel. */
	CMD_LOGGING_SOC_RESET_TRIGGERED,		/**< SoC reset has been triggered. */
	CMD_LOGGING_SOC_NMI_TRIGGERED,			/**< SoC NMI has been triggered. */
//	CMD_LOGGING_MCTP_ERR_MSG,				/**< MCTP error message received. */
	CMD_LOGGING_UNSEAL_FAIL = 8,			/**< An unseal operation failed. */
	CMD_LOGGING_RESTORE_BYPASS_FAIL,		/**< Failed to revert device to bypass mode. */
	CMD_LOGGING_BYPASS_RESTORED,			/**< Device has been reverted to bypass mode. */
	CMD_LOGGING_RESTORE_DEFAULTS_FAIL,		/**< Failed to revert device to the default state. */
	CMD_LOGGING_DEFAULTS_RESTORED,			/**< Device has been wiped of all configuration. */
	CMD_LOGGING_NOTIFICATION_ERROR,			/**< Unknown background task action specified. */
	CMD_LOGGING_DEBUG_LOG_CLEAR_FAIL,		/**< Failed to clear debug log. */
	CMD_LOGGING_COMMAND_TIMEOUT,			/**< Command response was not sent due to processing timeout. */
	CMD_LOGGING_DEBUG_LOG_CLEARED,			/**< The debug log has been cleared. */
};


#endif /* CMD_LOGGING_H_ */
