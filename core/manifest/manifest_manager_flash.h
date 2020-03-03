// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#ifndef MANIFEST_MANAGER_FLASH_H_
#define MANIFEST_MANAGER_FLASH_H_

#include <stdint.h>
#include "platform.h"
#include "manifest.h"
#include "state_manager/state_manager.h"
#include "crypto/hash.h"
#include "common/signature_verification.h"
#include "flash/spi_flash.h"
#include "flash/flash_updater.h"


/**
 * Container of information for each managed manifest region on flash.
 */
struct manifest_manager_flash_region {
	struct manifest *manifest;						/**< The flash region for the manifest. */
	int ref_count;									/**< The number of active references to the manifest region. */
	bool is_valid;									/**< Flag indicating if the region has a valid manifest. */
	struct flash_updater updater;					/**< Update manager for the flash region. */
};

/**
 * A manager for a single set of manifests stored in flash.
 */
struct manifest_manager_flash {
	struct manifest_manager_flash_region region1;	/**< The first flash region for a manifest. */
	struct manifest_manager_flash_region region2;	/**< The second flash region for a manifest. */
	struct state_manager *state;					/**< State manager interface. */
	struct hash_engine *hash;						/**< The hash engine for manifest validation. */
	struct signature_verification *verification;	/**< Verification module for verifying manifest signatures. */
	struct flash_updater *updating;					/**< The update manager being used to write new manifest data. */
	platform_mutex lock;							/**< Synchronization for flash manager state. */
	uint8_t manifest_index;							/**< Index of manifest in state manager. */

	/**
	 * Function called after standard manifest verification has been completed successfully.  This
	 * can be null to skip calling this function.
	 *
	 * @param manager The manager running verification.
	 *
	 * @return 0 if verification should complete successfully or an error code.
	 */
	int (*post_verify) (struct manifest_manager_flash *manager);
};


int manifest_manager_flash_init (struct manifest_manager_flash *manager, struct manifest *region1,
	struct manifest *region2, struct state_manager *state, struct hash_engine *hash,
	struct signature_verification *verification, struct spi_flash *region1_flash,
	uint32_t region1_addr, struct spi_flash *region2_flash, uint32_t region2_addr, 
	uint8_t manifest_index);
void manifest_manager_flash_release (struct manifest_manager_flash *manager);

struct manifest_manager_flash_region* manifest_manager_flash_get_region (
	struct manifest_manager_flash *manager, bool active);
struct manifest_manager_flash_region* manifest_manager_flash_get_manifest_region (
	struct manifest_manager_flash *manager, bool active);
void manifest_manager_flash_free_manifest (struct manifest_manager_flash *manager,
	struct manifest *manifest);

int manifest_manager_flash_activate_pending_manifest (struct manifest_manager_flash *manager);
int manifest_manager_flash_clear_pending_region (struct manifest_manager_flash *manager,
	size_t size);
int manifest_manager_flash_write_pending_data (struct manifest_manager_flash *manager,
	const uint8_t *data, size_t length);
int manifest_manager_flash_verify_pending_manifest (struct manifest_manager_flash *manager);
int manifest_manager_flash_clear_all_manifests (struct manifest_manager_flash *manager);


#endif /* MANIFEST_MANAGER_FLASH_H_ */
