// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "testing.h"
#include "host_processor_dual_testing.h"
#include "rsa_testing.h"


static const char *SUITE = "host_processor_dual";


/*******************
 * Test cases
 *******************/

static void host_processor_dual_test_flash_rollback_no_pfm (CuTest *test)
{
	struct host_processor_dual_testing host;
	struct flash_master_mock flash1_mock_host;
	struct flash_master_mock flash2_mock_host;
	struct spi_flash flash1_host;
	struct spi_flash flash2_host;
	int status;
	const int flash_size = 0x300;
	uint8_t data[flash_size];
	int i;

	TEST_START;

	for (i = 0; i < flash_size; i++) {
		data[i] = RSA_PRIVKEY_DER[i % RSA_PRIVKEY_DER_LEN];
	}

	host_processor_dual_testing_init (test, &host);
	host.flash_mock_state.mock.name = "flash_state";

	status = flash_master_mock_init (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash1_mock_host.mock.name = "flash1_mock_host";

	status = flash_master_mock_init (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash2_mock_host.mock.name = "flash2_mock_host";

	status = spi_flash_init (&flash1_host, &flash1_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash2_host, &flash2_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash1_host, flash_size);
	status |= spi_flash_set_device_size (&flash2_host, flash_size);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) NULL);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_only_flash,
		&host.flash_mgr, (intptr_t) &flash1_host);
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_write_flash,
		&host.flash_mgr, (intptr_t) &flash2_host);

	status |= flash_master_mock_expect_chip_erase (&flash1_mock_host);
	status |= flash_master_mock_expect_copy_flash_verify (&flash1_mock_host, &flash2_mock_host, 0,
		0, data, flash_size);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = flash_master_mock_validate_and_release (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);

	spi_flash_release (&flash1_host);
	spi_flash_release (&flash2_host);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_no_pfm_bypass (CuTest *test)
{
	struct host_processor_dual_testing host;
	struct flash_master_mock flash1_mock_host;
	struct flash_master_mock flash2_mock_host;
	struct spi_flash flash1_host;
	struct spi_flash flash2_host;
	int status;
	const int flash_size = 0x300;
	uint8_t data[flash_size];
	int i;

	TEST_START;

	for (i = 0; i < flash_size; i++) {
		data[i] = RSA_PRIVKEY_DER[i % RSA_PRIVKEY_DER_LEN];
	}

	host_processor_dual_testing_init (test, &host);
	host.flash_mock_state.mock.name = "flash_state";

	status = flash_master_mock_init (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash1_mock_host.mock.name = "flash1_mock_host";

	status = flash_master_mock_init (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash2_mock_host.mock.name = "flash2_mock_host";

	status = spi_flash_init (&flash1_host, &flash1_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash2_host, &flash2_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash1_host, flash_size);
	status |= spi_flash_set_device_size (&flash2_host, flash_size);
	CuAssertIntEquals (test, 0, status);

	host_state_manager_set_bypass_mode (&host.host_state, true);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) NULL);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_only_flash,
		&host.flash_mgr, (intptr_t) &flash1_host);
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_write_flash,
		&host.flash_mgr, (intptr_t) &flash2_host);

	status |= flash_master_mock_expect_chip_erase (&flash1_mock_host);
	status |= flash_master_mock_expect_copy_flash_verify (&flash1_mock_host, &flash2_mock_host, 0,
		0, data, flash_size);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = flash_master_mock_validate_and_release (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);

	spi_flash_release (&flash1_host);
	spi_flash_release (&flash2_host);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_no_pfm_checked (CuTest *test)
{
	struct host_processor_dual_testing host;
	struct flash_master_mock flash1_mock_host;
	struct flash_master_mock flash2_mock_host;
	struct spi_flash flash1_host;
	struct spi_flash flash2_host;
	int status;
	const int flash_size = 0x300;
	uint8_t data[flash_size];
	int i;

	TEST_START;

	for (i = 0; i < flash_size; i++) {
		data[i] = RSA_PRIVKEY_DER[i % RSA_PRIVKEY_DER_LEN];
	}

	host_processor_dual_testing_init (test, &host);
	host.flash_mock_state.mock.name = "flash_state";

	status = flash_master_mock_init (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash1_mock_host.mock.name = "flash1_mock_host";

	status = flash_master_mock_init (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash2_mock_host.mock.name = "flash2_mock_host";

	status = spi_flash_init (&flash1_host, &flash1_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash2_host, &flash2_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash1_host, flash_size);
	status |= spi_flash_set_device_size (&flash2_host, flash_size);
	CuAssertIntEquals (test, 0, status);

	host_state_manager_set_pfm_dirty (&host.host_state, false);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) NULL);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_only_flash,
		&host.flash_mgr, (intptr_t) &flash1_host);
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_write_flash,
		&host.flash_mgr, (intptr_t) &flash2_host);

	status |= flash_master_mock_expect_chip_erase (&flash1_mock_host);
	status |= flash_master_mock_expect_copy_flash_verify (&flash1_mock_host, &flash2_mock_host, 0,
		0, data, flash_size);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = flash_master_mock_validate_and_release (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);

	spi_flash_release (&flash1_host);
	spi_flash_release (&flash2_host);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_no_pfm_checked_bypass (CuTest *test)
{
	struct host_processor_dual_testing host;
	struct flash_master_mock flash1_mock_host;
	struct flash_master_mock flash2_mock_host;
	struct spi_flash flash1_host;
	struct spi_flash flash2_host;
	int status;
	const int flash_size = 0x300;
	uint8_t data[flash_size];
	int i;

	TEST_START;

	for (i = 0; i < flash_size; i++) {
		data[i] = RSA_PRIVKEY_DER[i % RSA_PRIVKEY_DER_LEN];
	}

	host_processor_dual_testing_init (test, &host);
	host.flash_mock_state.mock.name = "flash_state";

	status = flash_master_mock_init (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash1_mock_host.mock.name = "flash1_mock_host";

	status = flash_master_mock_init (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash2_mock_host.mock.name = "flash2_mock_host";

	status = spi_flash_init (&flash1_host, &flash1_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash2_host, &flash2_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash1_host, flash_size);
	status |= spi_flash_set_device_size (&flash2_host, flash_size);
	CuAssertIntEquals (test, 0, status);

	host_state_manager_set_pfm_dirty (&host.host_state, false);
	host_state_manager_set_bypass_mode (&host.host_state, true);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) NULL);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_only_flash,
		&host.flash_mgr, (intptr_t) &flash1_host);
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_write_flash,
		&host.flash_mgr, (intptr_t) &flash2_host);

	status |= flash_master_mock_expect_chip_erase (&flash1_mock_host);
	status |= flash_master_mock_expect_copy_flash_verify (&flash1_mock_host, &flash2_mock_host, 0,
		0, data, flash_size);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = flash_master_mock_validate_and_release (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);

	spi_flash_release (&flash1_host);
	spi_flash_release (&flash2_host);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_no_pfm_dirty (CuTest *test)
{
	struct host_processor_dual_testing host;
	struct flash_master_mock flash1_mock_host;
	struct flash_master_mock flash2_mock_host;
	struct spi_flash flash1_host;
	struct spi_flash flash2_host;
	int status;
	const int flash_size = 0x300;
	uint8_t data[flash_size];
	int i;

	TEST_START;

	for (i = 0; i < flash_size; i++) {
		data[i] = RSA_PRIVKEY_DER[i % RSA_PRIVKEY_DER_LEN];
	}

	host_processor_dual_testing_init (test, &host);
	host.flash_mock_state.mock.name = "flash_state";

	status = flash_master_mock_init (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash1_mock_host.mock.name = "flash1_mock_host";

	status = flash_master_mock_init (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash2_mock_host.mock.name = "flash2_mock_host";

	status = spi_flash_init (&flash1_host, &flash1_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash2_host, &flash2_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash1_host, flash_size);
	status |= spi_flash_set_device_size (&flash2_host, flash_size);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_save_inactive_dirty (&host.host_state, true);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) NULL);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_only_flash,
		&host.flash_mgr, (intptr_t) &flash1_host);
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_write_flash,
		&host.flash_mgr, (intptr_t) &flash2_host);

	status |= flash_master_mock_expect_chip_erase (&flash1_mock_host);
	status |= flash_master_mock_expect_copy_flash_verify (&flash1_mock_host, &flash2_mock_host, 0,
		0, data, flash_size);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_inactive_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = flash_master_mock_validate_and_release (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);

	spi_flash_release (&flash1_host);
	spi_flash_release (&flash2_host);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_no_pfm_dirty_bypass (CuTest *test)
{
	struct host_processor_dual_testing host;
	struct flash_master_mock flash1_mock_host;
	struct flash_master_mock flash2_mock_host;
	struct spi_flash flash1_host;
	struct spi_flash flash2_host;
	int status;
	const int flash_size = 0x300;
	uint8_t data[flash_size];
	int i;

	TEST_START;

	for (i = 0; i < flash_size; i++) {
		data[i] = RSA_PRIVKEY_DER[i % RSA_PRIVKEY_DER_LEN];
	}

	host_processor_dual_testing_init (test, &host);
	host.flash_mock_state.mock.name = "flash_state";

	status = flash_master_mock_init (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash1_mock_host.mock.name = "flash1_mock_host";

	status = flash_master_mock_init (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash2_mock_host.mock.name = "flash2_mock_host";

	status = spi_flash_init (&flash1_host, &flash1_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash2_host, &flash2_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash1_host, flash_size);
	status |= spi_flash_set_device_size (&flash2_host, flash_size);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_save_inactive_dirty (&host.host_state, true);
	CuAssertIntEquals (test, 0, status);

	host_state_manager_set_bypass_mode (&host.host_state, true);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) NULL);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_only_flash,
		&host.flash_mgr, (intptr_t) &flash1_host);
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_write_flash,
		&host.flash_mgr, (intptr_t) &flash2_host);

	status |= flash_master_mock_expect_chip_erase (&flash1_mock_host);
	status |= flash_master_mock_expect_copy_flash_verify (&flash1_mock_host, &flash2_mock_host, 0,
		0, data, flash_size);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_inactive_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = flash_master_mock_validate_and_release (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);

	spi_flash_release (&flash1_host);
	spi_flash_release (&flash2_host);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_no_pfm_dirty_checked (CuTest *test)
{
	struct host_processor_dual_testing host;
	struct flash_master_mock flash1_mock_host;
	struct flash_master_mock flash2_mock_host;
	struct spi_flash flash1_host;
	struct spi_flash flash2_host;
	int status;
	const int flash_size = 0x300;
	uint8_t data[flash_size];
	int i;

	TEST_START;

	for (i = 0; i < flash_size; i++) {
		data[i] = RSA_PRIVKEY_DER[i % RSA_PRIVKEY_DER_LEN];
	}

	host_processor_dual_testing_init (test, &host);
	host.flash_mock_state.mock.name = "flash_state";

	status = flash_master_mock_init (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash1_mock_host.mock.name = "flash1_mock_host";

	status = flash_master_mock_init (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash2_mock_host.mock.name = "flash2_mock_host";

	status = spi_flash_init (&flash1_host, &flash1_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash2_host, &flash2_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash1_host, flash_size);
	status |= spi_flash_set_device_size (&flash2_host, flash_size);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_save_inactive_dirty (&host.host_state, true);
	CuAssertIntEquals (test, 0, status);

	host_state_manager_set_pfm_dirty (&host.host_state, false);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) NULL);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_only_flash,
		&host.flash_mgr, (intptr_t) &flash1_host);
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_write_flash,
		&host.flash_mgr, (intptr_t) &flash2_host);

	status |= flash_master_mock_expect_chip_erase (&flash1_mock_host);
	status |= flash_master_mock_expect_copy_flash_verify (&flash1_mock_host, &flash2_mock_host, 0,
		0, data, flash_size);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_inactive_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = flash_master_mock_validate_and_release (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);

	spi_flash_release (&flash1_host);
	spi_flash_release (&flash2_host);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_no_pfm_dirty_checked_bypass (CuTest *test)
{
	struct host_processor_dual_testing host;
	struct flash_master_mock flash1_mock_host;
	struct flash_master_mock flash2_mock_host;
	struct spi_flash flash1_host;
	struct spi_flash flash2_host;
	int status;
	const int flash_size = 0x300;
	uint8_t data[flash_size];
	int i;

	TEST_START;

	for (i = 0; i < flash_size; i++) {
		data[i] = RSA_PRIVKEY_DER[i % RSA_PRIVKEY_DER_LEN];
	}

	host_processor_dual_testing_init (test, &host);
	host.flash_mock_state.mock.name = "flash_state";

	status = flash_master_mock_init (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash1_mock_host.mock.name = "flash1_mock_host";

	status = flash_master_mock_init (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash2_mock_host.mock.name = "flash2_mock_host";

	status = spi_flash_init (&flash1_host, &flash1_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash2_host, &flash2_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash1_host, flash_size);
	status |= spi_flash_set_device_size (&flash2_host, flash_size);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_save_inactive_dirty (&host.host_state, true);
	CuAssertIntEquals (test, 0, status);

	host_state_manager_set_pfm_dirty (&host.host_state, false);
	host_state_manager_set_bypass_mode (&host.host_state, true);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) NULL);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_only_flash,
		&host.flash_mgr, (intptr_t) &flash1_host);
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_write_flash,
		&host.flash_mgr, (intptr_t) &flash2_host);

	status |= flash_master_mock_expect_chip_erase (&flash1_mock_host);
	status |= flash_master_mock_expect_copy_flash_verify (&flash1_mock_host, &flash2_mock_host, 0,
		0, data, flash_size);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_inactive_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = flash_master_mock_validate_and_release (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);

	spi_flash_release (&flash1_host);
	spi_flash_release (&flash2_host);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_no_pfm_pulse_reset (CuTest *test)
{
	struct host_processor_dual_testing host;
	struct flash_master_mock flash1_mock_host;
	struct flash_master_mock flash2_mock_host;
	struct spi_flash flash1_host;
	struct spi_flash flash2_host;
	int status;
	const int flash_size = 0x300;
	uint8_t data[flash_size];
	int i;

	TEST_START;

	for (i = 0; i < flash_size; i++) {
		data[i] = RSA_PRIVKEY_DER[i % RSA_PRIVKEY_DER_LEN];
	}

	host_processor_dual_testing_init_pulse_reset (test, &host);
	host.flash_mock_state.mock.name = "flash_state";

	status = flash_master_mock_init (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash1_mock_host.mock.name = "flash1_mock_host";

	status = flash_master_mock_init (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash2_mock_host.mock.name = "flash2_mock_host";

	status = spi_flash_init (&flash1_host, &flash1_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash2_host, &flash2_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash1_host, flash_size);
	status |= spi_flash_set_device_size (&flash2_host, flash_size);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) NULL);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_only_flash,
		&host.flash_mgr, (intptr_t) &flash1_host);
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_write_flash,
		&host.flash_mgr, (intptr_t) &flash2_host);

	status |= flash_master_mock_expect_chip_erase (&flash1_mock_host);
	status |= flash_master_mock_expect_copy_flash_verify (&flash1_mock_host, &flash2_mock_host, 0,
		0, data, flash_size);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = flash_master_mock_validate_and_release (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);

	spi_flash_release (&flash1_host);
	spi_flash_release (&flash2_host);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_no_pfm_disable_bypass (CuTest *test)
{
	struct host_processor_dual_testing host;
	struct flash_master_mock flash1_mock_host;
	struct flash_master_mock flash2_mock_host;
	struct spi_flash flash1_host;
	struct spi_flash flash2_host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);
	host.flash_mock_state.mock.name = "flash_state";

	status = flash_master_mock_init (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash1_mock_host.mock.name = "flash1_mock_host";

	status = flash_master_mock_init (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash2_mock_host.mock.name = "flash2_mock_host";

	status = spi_flash_init (&flash1_host, &flash1_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash2_host, &flash2_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) NULL);

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base, true,
		false);
	CuAssertIntEquals (test, HOST_PROCESSOR_NO_ROLLBACK, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = flash_master_mock_validate_and_release (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);

	spi_flash_release (&flash1_host);
	spi_flash_release (&flash2_host);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_no_pfm_no_reset (CuTest *test)
{
	struct host_processor_dual_testing host;
	struct flash_master_mock flash1_mock_host;
	struct flash_master_mock flash2_mock_host;
	struct spi_flash flash1_host;
	struct spi_flash flash2_host;
	int status;
	const int flash_size = 0x300;
	uint8_t data[flash_size];
	int i;

	TEST_START;

	for (i = 0; i < flash_size; i++) {
		data[i] = RSA_PRIVKEY_DER[i % RSA_PRIVKEY_DER_LEN];
	}

	host_processor_dual_testing_init (test, &host);
	host.flash_mock_state.mock.name = "flash_state";

	status = flash_master_mock_init (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash1_mock_host.mock.name = "flash1_mock_host";

	status = flash_master_mock_init (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash2_mock_host.mock.name = "flash2_mock_host";

	status = spi_flash_init (&flash1_host, &flash1_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash2_host, &flash2_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash1_host, flash_size);
	status |= spi_flash_set_device_size (&flash2_host, flash_size);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) NULL);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_only_flash,
		&host.flash_mgr, (intptr_t) &flash1_host);
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_write_flash,
		&host.flash_mgr, (intptr_t) &flash2_host);

	status |= flash_master_mock_expect_chip_erase (&flash1_mock_host);
	status |= flash_master_mock_expect_copy_flash_verify (&flash1_mock_host, &flash2_mock_host, 0,
		0, data, flash_size);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, true);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = flash_master_mock_validate_and_release (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);

	spi_flash_release (&flash1_host);
	spi_flash_release (&flash2_host);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_no_pfm_no_reset_pulse_reset (CuTest *test)
{
	struct host_processor_dual_testing host;
	struct flash_master_mock flash1_mock_host;
	struct flash_master_mock flash2_mock_host;
	struct spi_flash flash1_host;
	struct spi_flash flash2_host;
	int status;
	const int flash_size = 0x300;
	uint8_t data[flash_size];
	int i;

	TEST_START;

	for (i = 0; i < flash_size; i++) {
		data[i] = RSA_PRIVKEY_DER[i % RSA_PRIVKEY_DER_LEN];
	}

	host_processor_dual_testing_init_pulse_reset (test, &host);
	host.flash_mock_state.mock.name = "flash_state";

	status = flash_master_mock_init (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash1_mock_host.mock.name = "flash1_mock_host";

	status = flash_master_mock_init (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash2_mock_host.mock.name = "flash2_mock_host";

	status = spi_flash_init (&flash1_host, &flash1_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash2_host, &flash2_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash1_host, flash_size);
	status |= spi_flash_set_device_size (&flash2_host, flash_size);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) NULL);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_only_flash,
		&host.flash_mgr, (intptr_t) &flash1_host);
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_write_flash,
		&host.flash_mgr, (intptr_t) &flash2_host);

	status |= flash_master_mock_expect_chip_erase (&flash1_mock_host);
	status |= flash_master_mock_expect_copy_flash_verify (&flash1_mock_host, &flash2_mock_host, 0,
		0, data, flash_size);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, true);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = flash_master_mock_validate_and_release (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);

	spi_flash_release (&flash1_host);
	spi_flash_release (&flash2_host);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_no_pfm_unsupported_flash (CuTest *test)
{
	struct host_processor_dual_testing host;
	struct flash_master_mock flash1_mock_host;
	struct flash_master_mock flash2_mock_host;
	struct spi_flash flash1_host;
	struct spi_flash flash2_host;
	int status;
	const int flash_size = 0x300;
	uint8_t data[flash_size];
	int i;

	TEST_START;

	for (i = 0; i < flash_size; i++) {
		data[i] = RSA_PRIVKEY_DER[i % RSA_PRIVKEY_DER_LEN];
	}

	host_processor_dual_testing_init (test, &host);
	host.flash_mock_state.mock.name = "flash_state";

	status = flash_master_mock_init (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash1_mock_host.mock.name = "flash1_mock_host";

	status = flash_master_mock_init (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash2_mock_host.mock.name = "flash2_mock_host";

	status = spi_flash_init (&flash1_host, &flash1_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash2_host, &flash2_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash1_host, flash_size);
	status |= spi_flash_set_device_size (&flash2_host, flash_size);
	CuAssertIntEquals (test, 0, status);

	host_state_manager_set_unsupported_flash (&host.host_state, true);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) NULL);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_only_flash,
		&host.flash_mgr, (intptr_t) &flash1_host);
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_write_flash,
		&host.flash_mgr, (intptr_t) &flash2_host);

	status |= flash_master_mock_expect_chip_erase (&flash1_mock_host);
	status |= flash_master_mock_expect_copy_flash_verify (&flash1_mock_host, &flash2_mock_host, 0,
		0, data, flash_size);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = flash_master_mock_validate_and_release (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);

	spi_flash_release (&flash1_host);
	spi_flash_release (&flash2_host);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty (CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;
	struct flash_region rw_region;
	struct pfm_read_write_regions rw_list;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	rw_region.start_addr = 0x200;
	rw_region.length = 0x100;

	rw_list.regions = &rw_region;
	rw_list.count = 1;

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.validate_read_write_flash,
		&host.flash_mgr, 0, MOCK_ARG (&host.pfm), MOCK_ARG (&host.hash), MOCK_ARG (&host.rsa),
		MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&host.flash_mgr.mock, 3, &rw_list, sizeof (rw_list), -1);
	status |= mock_expect_save_arg (&host.flash_mgr.mock, 3, 0);
	status |= mock_expect_share_save_arg (&host.flash_mgr.mock, 0, &host.pfm.mock, 0);

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region, &host.filter,
		0, MOCK_ARG (1), MOCK_ARG (0x200), MOCK_ARG (0x300));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.swap_flash_devices,
		&host.flash_mgr, 0, MOCK_ARG (NULL), MOCK_ARG (NULL));

	status |= mock_expect (&host.observer.mock, host.observer.base.on_active_mode, &host.observer,
		0);

	status |= mock_expect (&host.pfm.mock, host.pfm.base.free_read_write_regions, &host.pfm, 0,
		MOCK_ARG_SAVED_ARG (0));

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_bypass (CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	host_state_manager_set_bypass_mode (&host.host_state, true);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region,
		&host.filter, 0, MOCK_ARG (1), MOCK_ARG (0), MOCK_ARG (0xffff0000));

	status |= mock_expect (&host.filter.mock, host.filter.base.set_ro_cs, &host.filter, 0,
		MOCK_ARG (SPI_FILTER_CS_0));

	status |= mock_expect (&host.observer.mock, host.observer.base.on_bypass_mode, &host.observer,
		0);

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_get_read_only_flash (&host.host_state);
	CuAssertIntEquals (test, SPI_FILTER_CS_1, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_bypass_cs1 (CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	status = host_state_manager_save_read_only_flash (&host.host_state, SPI_FILTER_CS_1);
	CuAssertIntEquals (test, 0, status);

	host_state_manager_set_bypass_mode (&host.host_state, true);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region,
		&host.filter, 0, MOCK_ARG (1), MOCK_ARG (0), MOCK_ARG (0xffff0000));

	status |= mock_expect (&host.filter.mock, host.filter.base.set_ro_cs, &host.filter, 0,
		MOCK_ARG (SPI_FILTER_CS_1));

	status |= mock_expect (&host.observer.mock, host.observer.base.on_bypass_mode, &host.observer,
		0);

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_get_read_only_flash (&host.host_state);
	CuAssertIntEquals (test, SPI_FILTER_CS_0, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_bypass_no_observer (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	status = host_processor_remove_observer (&host.test.base, &host.observer.base);
	CuAssertIntEquals (test, 0, status);

	host_state_manager_set_bypass_mode (&host.host_state, true);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region,
		&host.filter, 0, MOCK_ARG (1), MOCK_ARG (0), MOCK_ARG (0xffff0000));

	status |= mock_expect (&host.filter.mock, host.filter.base.set_ro_cs, &host.filter, 0,
		MOCK_ARG (SPI_FILTER_CS_0));

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_get_read_only_flash (&host.host_state);
	CuAssertIntEquals (test, SPI_FILTER_CS_1, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_checked (CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;
	struct flash_region rw_region;
	struct pfm_read_write_regions rw_list;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	host_state_manager_set_pfm_dirty (&host.host_state, false);

	rw_region.start_addr = 0x200;
	rw_region.length = 0x100;

	rw_list.regions = &rw_region;
	rw_list.count = 1;

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.validate_read_write_flash,
		&host.flash_mgr, 0, MOCK_ARG (&host.pfm), MOCK_ARG (&host.hash), MOCK_ARG (&host.rsa),
		MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&host.flash_mgr.mock, 3, &rw_list, sizeof (rw_list), -1);
	status |= mock_expect_save_arg (&host.flash_mgr.mock, 3, 0);
	status |= mock_expect_share_save_arg (&host.flash_mgr.mock, 0, &host.pfm.mock, 0);

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region, &host.filter,
		0, MOCK_ARG (1), MOCK_ARG (0x200), MOCK_ARG (0x300));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.swap_flash_devices,
		&host.flash_mgr, 0, MOCK_ARG (NULL), MOCK_ARG (NULL));

	status |= mock_expect (&host.observer.mock, host.observer.base.on_active_mode, &host.observer,
		0);

	status |= mock_expect (&host.pfm.mock, host.pfm.base.free_read_write_regions, &host.pfm, 0,
		MOCK_ARG_SAVED_ARG (0));

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_checked_bypass (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	host_state_manager_set_pfm_dirty (&host.host_state, false);
	host_state_manager_set_bypass_mode (&host.host_state, true);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region,
		&host.filter, 0, MOCK_ARG (1), MOCK_ARG (0), MOCK_ARG (0xffff0000));

	status |= mock_expect (&host.filter.mock, host.filter.base.set_ro_cs, &host.filter, 0,
		MOCK_ARG (SPI_FILTER_CS_0));

	status |= mock_expect (&host.observer.mock, host.observer.base.on_bypass_mode, &host.observer,
		0);

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_get_read_only_flash (&host.host_state);
	CuAssertIntEquals (test, SPI_FILTER_CS_1, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_pulse_reset (CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;
	struct flash_region rw_region;
	struct pfm_read_write_regions rw_list;

	TEST_START;

	host_processor_dual_testing_init_pulse_reset (test, &host);

	rw_region.start_addr = 0x200;
	rw_region.length = 0x100;

	rw_list.regions = &rw_region;
	rw_list.count = 1;

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.validate_read_write_flash,
		&host.flash_mgr, 0, MOCK_ARG (&host.pfm), MOCK_ARG (&host.hash), MOCK_ARG (&host.rsa),
		MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&host.flash_mgr.mock, 3, &rw_list, sizeof (rw_list), -1);
	status |= mock_expect_save_arg (&host.flash_mgr.mock, 3, 0);
	status |= mock_expect_share_save_arg (&host.flash_mgr.mock, 0, &host.pfm.mock, 0);

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region, &host.filter,
		0, MOCK_ARG (1), MOCK_ARG (0x200), MOCK_ARG (0x300));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.swap_flash_devices,
		&host.flash_mgr, 0, MOCK_ARG (NULL), MOCK_ARG (NULL));

	status |= mock_expect (&host.observer.mock, host.observer.base.on_active_mode, &host.observer,
		0);

	status |= mock_expect (&host.pfm.mock, host.pfm.base.free_read_write_regions, &host.pfm, 0,
		MOCK_ARG_SAVED_ARG (0));

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_bypass_disable_bypass (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	host_state_manager_set_bypass_mode (&host.host_state, true);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base, true,
		false);
	CuAssertIntEquals (test, HOST_PROCESSOR_NO_ROLLBACK, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_get_read_only_flash (&host.host_state);
	CuAssertIntEquals (test, SPI_FILTER_CS_0, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_no_reset (CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;
	struct flash_region rw_region;
	struct pfm_read_write_regions rw_list;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	rw_region.start_addr = 0x200;
	rw_region.length = 0x100;

	rw_list.regions = &rw_region;
	rw_list.count = 1;

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.validate_read_write_flash,
		&host.flash_mgr, 0, MOCK_ARG (&host.pfm), MOCK_ARG (&host.hash), MOCK_ARG (&host.rsa),
		MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&host.flash_mgr.mock, 3, &rw_list, sizeof (rw_list), -1);
	status |= mock_expect_save_arg (&host.flash_mgr.mock, 3, 0);
	status |= mock_expect_share_save_arg (&host.flash_mgr.mock, 0, &host.pfm.mock, 0);

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region, &host.filter,
		0, MOCK_ARG (1), MOCK_ARG (0x200), MOCK_ARG (0x300));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.swap_flash_devices,
		&host.flash_mgr, 0, MOCK_ARG (NULL), MOCK_ARG (NULL));

	status |= mock_expect (&host.observer.mock, host.observer.base.on_active_mode, &host.observer,
		0);

	status |= mock_expect (&host.pfm.mock, host.pfm.base.free_read_write_regions, &host.pfm, 0,
		MOCK_ARG_SAVED_ARG (0));

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, true);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_no_reset_pulse_reset (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;
	struct flash_region rw_region;
	struct pfm_read_write_regions rw_list;

	TEST_START;

	host_processor_dual_testing_init_pulse_reset (test, &host);

	rw_region.start_addr = 0x200;
	rw_region.length = 0x100;

	rw_list.regions = &rw_region;
	rw_list.count = 1;

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.validate_read_write_flash,
		&host.flash_mgr, 0, MOCK_ARG (&host.pfm), MOCK_ARG (&host.hash), MOCK_ARG (&host.rsa),
		MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&host.flash_mgr.mock, 3, &rw_list, sizeof (rw_list), -1);
	status |= mock_expect_save_arg (&host.flash_mgr.mock, 3, 0);
	status |= mock_expect_share_save_arg (&host.flash_mgr.mock, 0, &host.pfm.mock, 0);

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region, &host.filter,
		0, MOCK_ARG (1), MOCK_ARG (0x200), MOCK_ARG (0x300));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.swap_flash_devices,
		&host.flash_mgr, 0, MOCK_ARG (NULL), MOCK_ARG (NULL));

	status |= mock_expect (&host.observer.mock, host.observer.base.on_active_mode, &host.observer,
		0);

	status |= mock_expect (&host.pfm.mock, host.pfm.base.free_read_write_regions, &host.pfm, 0,
		MOCK_ARG_SAVED_ARG (0));

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, true);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_unsupported_flash (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	host_state_manager_set_unsupported_flash (&host.host_state, true);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);
	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, HOST_PROCESSOR_FLASH_NOT_SUPPORTED, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_dirty (CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	status = host_state_manager_save_inactive_dirty (&host.host_state, true);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);
	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, HOST_PROCESSOR_ROLLBACK_DIRTY, status);

	status = host_state_manager_is_inactive_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_dirty_bypass (CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	status = host_state_manager_save_inactive_dirty (&host.host_state, true);
	CuAssertIntEquals (test, 0, status);

	host_state_manager_set_bypass_mode (&host.host_state, true);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region,
		&host.filter, 0, MOCK_ARG (1), MOCK_ARG (0), MOCK_ARG (0xffff0000));

	status |= mock_expect (&host.filter.mock, host.filter.base.set_ro_cs, &host.filter, 0,
		MOCK_ARG (SPI_FILTER_CS_0));

	status |= mock_expect (&host.observer.mock, host.observer.base.on_bypass_mode, &host.observer,
		0);

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_inactive_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_get_read_only_flash (&host.host_state);
	CuAssertIntEquals (test, SPI_FILTER_CS_1, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_dirty_checked (CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	status = host_state_manager_save_inactive_dirty (&host.host_state, true);
	CuAssertIntEquals (test, 0, status);

	host_state_manager_set_pfm_dirty (&host.host_state, false);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);
	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, HOST_PROCESSOR_ROLLBACK_DIRTY, status);

	status = host_state_manager_is_inactive_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_dirty_checked_bypass (CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	status = host_state_manager_save_inactive_dirty (&host.host_state, true);
	CuAssertIntEquals (test, 0, status);

	host_state_manager_set_pfm_dirty (&host.host_state, false);
	host_state_manager_set_bypass_mode (&host.host_state, true);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region,
		&host.filter, 0, MOCK_ARG (1), MOCK_ARG (0), MOCK_ARG (0xffff0000));

	status |= mock_expect (&host.filter.mock, host.filter.base.set_ro_cs, &host.filter, 0,
		MOCK_ARG (SPI_FILTER_CS_0));

	status |= mock_expect (&host.observer.mock, host.observer.base.on_bypass_mode, &host.observer,
		0);

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_inactive_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_get_read_only_flash (&host.host_state);
	CuAssertIntEquals (test, SPI_FILTER_CS_1, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_dirty_bypass_disable_bypass (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	status = host_state_manager_save_inactive_dirty (&host.host_state, true);
	CuAssertIntEquals (test, 0, status);

	host_state_manager_set_bypass_mode (&host.host_state, true);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base, true,
		false);
	CuAssertIntEquals (test, HOST_PROCESSOR_NO_ROLLBACK, status);

	status = host_state_manager_is_inactive_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_get_read_only_flash (&host.host_state);
	CuAssertIntEquals (test, SPI_FILTER_CS_0, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_dirty_bypass_no_reset (CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	status = host_state_manager_save_inactive_dirty (&host.host_state, true);
	CuAssertIntEquals (test, 0, status);

	host_state_manager_set_bypass_mode (&host.host_state, true);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region,
		&host.filter, 0, MOCK_ARG (1), MOCK_ARG (0), MOCK_ARG (0xffff0000));

	status |= mock_expect (&host.filter.mock, host.filter.base.set_ro_cs, &host.filter, 0,
		MOCK_ARG (SPI_FILTER_CS_0));

	status |= mock_expect (&host.observer.mock, host.observer.base.on_bypass_mode, &host.observer,
		0);

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, true);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_inactive_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_get_read_only_flash (&host.host_state);
	CuAssertIntEquals (test, SPI_FILTER_CS_1, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_dirty_bypass_no_reset_pulse_reset (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init_pulse_reset (test, &host);

	status = host_state_manager_save_inactive_dirty (&host.host_state, true);
	CuAssertIntEquals (test, 0, status);

	host_state_manager_set_bypass_mode (&host.host_state, true);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region,
		&host.filter, 0, MOCK_ARG (1), MOCK_ARG (0), MOCK_ARG (0xffff0000));

	status |= mock_expect (&host.filter.mock, host.filter.base.set_ro_cs, &host.filter, 0,
		MOCK_ARG (SPI_FILTER_CS_0));

	status |= mock_expect (&host.observer.mock, host.observer.base.on_bypass_mode, &host.observer,
		0);

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, true);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_inactive_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_get_read_only_flash (&host.host_state);
	CuAssertIntEquals (test, SPI_FILTER_CS_1, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_dirty_unsupported_flash (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	status = host_state_manager_save_inactive_dirty (&host.host_state, true);
	CuAssertIntEquals (test, 0, status);

	host_state_manager_set_unsupported_flash (&host.host_state, true);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);
	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, HOST_PROCESSOR_FLASH_NOT_SUPPORTED, status);

	status = host_state_manager_is_inactive_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_validation_fail (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.validate_read_write_flash,
		&host.flash_mgr, RSA_ENGINE_BAD_SIGNATURE, MOCK_ARG (&host.pfm), MOCK_ARG (&host.hash),
		MOCK_ARG (&host.rsa),
		MOCK_ARG_NOT_NULL);

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, RSA_ENGINE_BAD_SIGNATURE, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_hash_validation_fail (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.validate_read_write_flash,
		&host.flash_mgr, HOST_FW_UTIL_BAD_IMAGE_HASH, MOCK_ARG (&host.pfm), MOCK_ARG (&host.hash),
		MOCK_ARG (&host.rsa),
		MOCK_ARG_NOT_NULL);

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, HOST_FW_UTIL_BAD_IMAGE_HASH, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_unknown_version (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.validate_read_write_flash,
		&host.flash_mgr, HOST_FW_UTIL_UNSUPPORTED_VERSION, MOCK_ARG (&host.pfm),
		MOCK_ARG (&host.hash), MOCK_ARG (&host.rsa),
		MOCK_ARG_NOT_NULL);

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, HOST_FW_UTIL_UNSUPPORTED_VERSION, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_blank_fail (CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.validate_read_write_flash,
		&host.flash_mgr, FLASH_UTIL_UNEXPECTED_VALUE, MOCK_ARG (&host.pfm), MOCK_ARG (&host.hash),
		MOCK_ARG (&host.rsa),
		MOCK_ARG_NOT_NULL);

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, FLASH_UTIL_UNEXPECTED_VALUE, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_validation_fail_pulse_reset (CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init_pulse_reset (test, &host);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.validate_read_write_flash,
		&host.flash_mgr, RSA_ENGINE_BAD_SIGNATURE, MOCK_ARG (&host.pfm), MOCK_ARG (&host.hash),
		MOCK_ARG (&host.rsa),
		MOCK_ARG_NOT_NULL);

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, RSA_ENGINE_BAD_SIGNATURE, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_hash_validation_fail_pulse_reset (CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init_pulse_reset (test, &host);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.validate_read_write_flash,
		&host.flash_mgr, HOST_FW_UTIL_BAD_IMAGE_HASH, MOCK_ARG (&host.pfm), MOCK_ARG (&host.hash),
		MOCK_ARG (&host.rsa),
		MOCK_ARG_NOT_NULL);

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, HOST_FW_UTIL_BAD_IMAGE_HASH, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_unknown_version_pulse_reset (CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init_pulse_reset (test, &host);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.validate_read_write_flash,
		&host.flash_mgr, HOST_FW_UTIL_UNSUPPORTED_VERSION, MOCK_ARG (&host.pfm),
		MOCK_ARG (&host.hash), MOCK_ARG (&host.rsa),
		MOCK_ARG_NOT_NULL);

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, HOST_FW_UTIL_UNSUPPORTED_VERSION, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_blank_fail_pulse_reset (CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init_pulse_reset (test, &host);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.validate_read_write_flash,
		&host.flash_mgr, FLASH_UTIL_UNEXPECTED_VALUE, MOCK_ARG (&host.pfm), MOCK_ARG (&host.hash),
		MOCK_ARG (&host.rsa),
		MOCK_ARG_NOT_NULL);

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, FLASH_UTIL_UNEXPECTED_VALUE, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_null (CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	status = host.test.base.flash_rollback (NULL, &host.hash.base, &host.rsa.base, false, false);
	CuAssertIntEquals (test, HOST_PROCESSOR_INVALID_ARGUMENT, status);

	status = host.test.base.flash_rollback (&host.test.base, NULL, &host.rsa.base, false, false);
	CuAssertIntEquals (test, HOST_PROCESSOR_INVALID_ARGUMENT, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, NULL, false, false);
	CuAssertIntEquals (test, HOST_PROCESSOR_INVALID_ARGUMENT, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_no_pfm_rot_access_error (CuTest *test)
{
	struct host_processor_dual_testing host;
	struct flash_master_mock flash1_mock_host;
	struct flash_master_mock flash2_mock_host;
	struct spi_flash flash1_host;
	struct spi_flash flash2_host;
	int status;
	const int flash_size = 0x300;

	TEST_START;

	host_processor_dual_testing_init (test, &host);
	host.flash_mock_state.mock.name = "flash_state";

	status = flash_master_mock_init (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash1_mock_host.mock.name = "flash1_mock_host";

	status = flash_master_mock_init (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash2_mock_host.mock.name = "flash2_mock_host";

	status = spi_flash_init (&flash1_host, &flash1_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash2_host, &flash2_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash1_host, flash_size);
	status |= spi_flash_set_device_size (&flash2_host, flash_size);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) NULL);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, HOST_FLASH_MGR_ROT_ACCESS_FAILED, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, HOST_FLASH_MGR_ROT_ACCESS_FAILED, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = flash_master_mock_validate_and_release (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);

	spi_flash_release (&flash1_host);
	spi_flash_release (&flash2_host);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_no_pfm_rot_access_error_pulse_reset (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	struct flash_master_mock flash1_mock_host;
	struct flash_master_mock flash2_mock_host;
	struct spi_flash flash1_host;
	struct spi_flash flash2_host;
	int status;
	const int flash_size = 0x300;

	TEST_START;

	host_processor_dual_testing_init_pulse_reset (test, &host);
	host.flash_mock_state.mock.name = "flash_state";

	status = flash_master_mock_init (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash1_mock_host.mock.name = "flash1_mock_host";

	status = flash_master_mock_init (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash2_mock_host.mock.name = "flash2_mock_host";

	status = spi_flash_init (&flash1_host, &flash1_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash2_host, &flash2_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash1_host, flash_size);
	status |= spi_flash_set_device_size (&flash2_host, flash_size);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) NULL);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, HOST_FLASH_MGR_ROT_ACCESS_FAILED, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, HOST_FLASH_MGR_ROT_ACCESS_FAILED, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = flash_master_mock_validate_and_release (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);

	spi_flash_release (&flash1_host);
	spi_flash_release (&flash2_host);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_no_pfm_rot_access_error_host_access_error (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	struct flash_master_mock flash1_mock_host;
	struct flash_master_mock flash2_mock_host;
	struct spi_flash flash1_host;
	struct spi_flash flash2_host;
	int status;
	const int flash_size = 0x300;

	TEST_START;

	host_processor_dual_testing_init (test, &host);
	host.flash_mock_state.mock.name = "flash_state";

	status = flash_master_mock_init (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash1_mock_host.mock.name = "flash1_mock_host";

	status = flash_master_mock_init (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash2_mock_host.mock.name = "flash2_mock_host";

	status = spi_flash_init (&flash1_host, &flash1_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash2_host, &flash2_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash1_host, flash_size);
	status |= spi_flash_set_device_size (&flash2_host, flash_size);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) NULL);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, HOST_FLASH_MGR_ROT_ACCESS_FAILED, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, HOST_FLASH_MGR_ROT_ACCESS_FAILED, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = flash_master_mock_validate_and_release (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);

	spi_flash_release (&flash1_host);
	spi_flash_release (&flash2_host);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_no_pfm_erase_error (CuTest *test)
{
	struct host_processor_dual_testing host;
	struct flash_master_mock flash1_mock_host;
	struct flash_master_mock flash2_mock_host;
	struct spi_flash flash1_host;
	struct spi_flash flash2_host;
	int status;
	const int flash_size = 0x300;

	TEST_START;

	host_processor_dual_testing_init (test, &host);
	host.flash_mock_state.mock.name = "flash_state";

	status = flash_master_mock_init (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash1_mock_host.mock.name = "flash1_mock_host";

	status = flash_master_mock_init (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash2_mock_host.mock.name = "flash2_mock_host";

	status = spi_flash_init (&flash1_host, &flash1_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash2_host, &flash2_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash1_host, flash_size);
	status |= spi_flash_set_device_size (&flash2_host, flash_size);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) NULL);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_only_flash,
		&host.flash_mgr, (intptr_t) &flash1_host);
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_write_flash,
		&host.flash_mgr, (intptr_t) &flash2_host);

	status |= flash_master_mock_expect_xfer (&flash1_mock_host, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_READ_STATUS_REG);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, FLASH_MASTER_XFER_FAILED, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = flash_master_mock_validate_and_release (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);

	spi_flash_release (&flash1_host);
	spi_flash_release (&flash2_host);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_no_pfm_erase_error_pulse_reset (CuTest *test)
{
	struct host_processor_dual_testing host;
	struct flash_master_mock flash1_mock_host;
	struct flash_master_mock flash2_mock_host;
	struct spi_flash flash1_host;
	struct spi_flash flash2_host;
	int status;
	const int flash_size = 0x300;

	TEST_START;

	host_processor_dual_testing_init_pulse_reset (test, &host);
	host.flash_mock_state.mock.name = "flash_state";

	status = flash_master_mock_init (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash1_mock_host.mock.name = "flash1_mock_host";

	status = flash_master_mock_init (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash2_mock_host.mock.name = "flash2_mock_host";

	status = spi_flash_init (&flash1_host, &flash1_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash2_host, &flash2_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash1_host, flash_size);
	status |= spi_flash_set_device_size (&flash2_host, flash_size);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) NULL);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_only_flash,
		&host.flash_mgr, (intptr_t) &flash1_host);
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_write_flash,
		&host.flash_mgr, (intptr_t) &flash2_host);

	status |= flash_master_mock_expect_xfer (&flash1_mock_host, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_READ_STATUS_REG);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, FLASH_MASTER_XFER_FAILED, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = flash_master_mock_validate_and_release (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);

	spi_flash_release (&flash1_host);
	spi_flash_release (&flash2_host);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_no_pfm_copy_error (CuTest *test)
{
	struct host_processor_dual_testing host;
	struct flash_master_mock flash1_mock_host;
	struct flash_master_mock flash2_mock_host;
	struct spi_flash flash1_host;
	struct spi_flash flash2_host;
	int status;
	const int flash_size = 0x300;

	TEST_START;

	host_processor_dual_testing_init (test, &host);
	host.flash_mock_state.mock.name = "flash_state";

	status = flash_master_mock_init (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash1_mock_host.mock.name = "flash1_mock_host";

	status = flash_master_mock_init (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash2_mock_host.mock.name = "flash2_mock_host";

	status = spi_flash_init (&flash1_host, &flash1_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash2_host, &flash2_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash1_host, flash_size);
	status |= spi_flash_set_device_size (&flash2_host, flash_size);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) NULL);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_only_flash,
		&host.flash_mgr, (intptr_t) &flash1_host);
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_write_flash,
		&host.flash_mgr, (intptr_t) &flash2_host);

	status |= flash_master_mock_expect_chip_erase (&flash1_mock_host);
	status |= flash_master_mock_expect_xfer (&flash2_mock_host, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_READ_STATUS_REG);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, FLASH_MASTER_XFER_FAILED, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = flash_master_mock_validate_and_release (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);

	spi_flash_release (&flash1_host);
	spi_flash_release (&flash2_host);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_no_pfm_copy_error_pulse_reset (CuTest *test)
{
	struct host_processor_dual_testing host;
	struct flash_master_mock flash1_mock_host;
	struct flash_master_mock flash2_mock_host;
	struct spi_flash flash1_host;
	struct spi_flash flash2_host;
	int status;
	const int flash_size = 0x300;

	TEST_START;

	host_processor_dual_testing_init_pulse_reset (test, &host);
	host.flash_mock_state.mock.name = "flash_state";

	status = flash_master_mock_init (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash1_mock_host.mock.name = "flash1_mock_host";

	status = flash_master_mock_init (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash2_mock_host.mock.name = "flash2_mock_host";

	status = spi_flash_init (&flash1_host, &flash1_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash2_host, &flash2_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash1_host, flash_size);
	status |= spi_flash_set_device_size (&flash2_host, flash_size);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) NULL);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_only_flash,
		&host.flash_mgr, (intptr_t) &flash1_host);
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_write_flash,
		&host.flash_mgr, (intptr_t) &flash2_host);

	status |= flash_master_mock_expect_chip_erase (&flash1_mock_host);
	status |= flash_master_mock_expect_xfer (&flash2_mock_host, FLASH_MASTER_XFER_FAILED,
		FLASH_EXP_READ_STATUS_REG);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, FLASH_MASTER_XFER_FAILED, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = flash_master_mock_validate_and_release (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);

	spi_flash_release (&flash1_host);
	spi_flash_release (&flash2_host);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_no_pfm_host_access_error (CuTest *test)
{
	struct host_processor_dual_testing host;
	struct flash_master_mock flash1_mock_host;
	struct flash_master_mock flash2_mock_host;
	struct spi_flash flash1_host;
	struct spi_flash flash2_host;
	int status;
	const int flash_size = 0x300;
	uint8_t data[flash_size];
	int i;

	TEST_START;

	for (i = 0; i < flash_size; i++) {
		data[i] = RSA_PRIVKEY_DER[i % RSA_PRIVKEY_DER_LEN];
	}

	host_processor_dual_testing_init (test, &host);
	host.flash_mock_state.mock.name = "flash_state";

	status = flash_master_mock_init (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash1_mock_host.mock.name = "flash1_mock_host";

	status = flash_master_mock_init (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash2_mock_host.mock.name = "flash2_mock_host";

	status = spi_flash_init (&flash1_host, &flash1_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash2_host, &flash2_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash1_host, flash_size);
	status |= spi_flash_set_device_size (&flash2_host, flash_size);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) NULL);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_only_flash,
		&host.flash_mgr, (intptr_t) &flash1_host);
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_write_flash,
		&host.flash_mgr, (intptr_t) &flash2_host);

	status |= flash_master_mock_expect_chip_erase (&flash1_mock_host);
	status |= flash_master_mock_expect_copy_flash_verify (&flash1_mock_host, &flash2_mock_host, 0,
		0, data, flash_size);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = flash_master_mock_validate_and_release (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);

	spi_flash_release (&flash1_host);
	spi_flash_release (&flash2_host);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_no_pfm_host_access_error_pulse_reset (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	struct flash_master_mock flash1_mock_host;
	struct flash_master_mock flash2_mock_host;
	struct spi_flash flash1_host;
	struct spi_flash flash2_host;
	int status;
	const int flash_size = 0x300;
	uint8_t data[flash_size];
	int i;

	TEST_START;

	for (i = 0; i < flash_size; i++) {
		data[i] = RSA_PRIVKEY_DER[i % RSA_PRIVKEY_DER_LEN];
	}

	host_processor_dual_testing_init_pulse_reset (test, &host);
	host.flash_mock_state.mock.name = "flash_state";

	status = flash_master_mock_init (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash1_mock_host.mock.name = "flash1_mock_host";

	status = flash_master_mock_init (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);
	flash2_mock_host.mock.name = "flash2_mock_host";

	status = spi_flash_init (&flash1_host, &flash1_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_init (&flash2_host, &flash2_mock_host.base);
	CuAssertIntEquals (test, 0, status);

	status = spi_flash_set_device_size (&flash1_host, flash_size);
	status |= spi_flash_set_device_size (&flash2_host, flash_size);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) NULL);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_only_flash,
		&host.flash_mgr, (intptr_t) &flash1_host);
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.get_read_write_flash,
		&host.flash_mgr, (intptr_t) &flash2_host);

	status |= flash_master_mock_expect_chip_erase (&flash1_mock_host);
	status |= flash_master_mock_expect_copy_flash_verify (&flash1_mock_host, &flash2_mock_host, 0,
		0, data, flash_size);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	status = flash_master_mock_validate_and_release (&flash1_mock_host);
	CuAssertIntEquals (test, 0, status);

	status = flash_master_mock_validate_and_release (&flash2_mock_host);
	CuAssertIntEquals (test, 0, status);

	spi_flash_release (&flash1_host);
	spi_flash_release (&flash2_host);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_rot_access_error (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, HOST_FLASH_MGR_ROT_ACCESS_FAILED, MOCK_ARG (&host.control));

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, HOST_FLASH_MGR_ROT_ACCESS_FAILED, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_rot_access_error_pulse_reset (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init_pulse_reset (test, &host);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, HOST_FLASH_MGR_ROT_ACCESS_FAILED, MOCK_ARG (&host.control));

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, HOST_FLASH_MGR_ROT_ACCESS_FAILED, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_rot_access_error_host_access_error (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, HOST_FLASH_MGR_ROT_ACCESS_FAILED, MOCK_ARG (&host.control));

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, HOST_FLASH_MGR_ROT_ACCESS_FAILED, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_validation_error (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.validate_read_write_flash,
		&host.flash_mgr, HOST_FLASH_MGR_VALIDATE_RW_FAILED, MOCK_ARG (&host.pfm),
		MOCK_ARG (&host.hash), MOCK_ARG (&host.rsa),
		MOCK_ARG_NOT_NULL);

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, HOST_FLASH_MGR_VALIDATE_RW_FAILED, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_validation_error_pulse_reset (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init_pulse_reset (test, &host);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.validate_read_write_flash,
		&host.flash_mgr, HOST_FLASH_MGR_VALIDATE_RW_FAILED, MOCK_ARG (&host.pfm),
		MOCK_ARG (&host.hash), MOCK_ARG (&host.rsa),
		MOCK_ARG_NOT_NULL);

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, HOST_FLASH_MGR_VALIDATE_RW_FAILED, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_swap_error (CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;
	struct flash_region rw_region;
	struct pfm_read_write_regions rw_list;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	rw_region.start_addr = 0x200;
	rw_region.length = 0x100;

	rw_list.regions = &rw_region;
	rw_list.count = 1;

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.validate_read_write_flash,
		&host.flash_mgr, 0, MOCK_ARG (&host.pfm), MOCK_ARG (&host.hash), MOCK_ARG (&host.rsa),
		MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&host.flash_mgr.mock, 3, &rw_list, sizeof (rw_list), -1);
	status |= mock_expect_save_arg (&host.flash_mgr.mock, 3, 0);
	status |= mock_expect_share_save_arg (&host.flash_mgr.mock, 0, &host.pfm.mock, 0);

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region, &host.filter,
		0, MOCK_ARG (1), MOCK_ARG (0x200), MOCK_ARG (0x300));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.swap_flash_devices,
		&host.flash_mgr, HOST_FLASH_MGR_SWAP_FAILED, MOCK_ARG (NULL), MOCK_ARG (NULL));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.swap_flash_devices,
		&host.flash_mgr, HOST_FLASH_MGR_SWAP_FAILED, MOCK_ARG (NULL), MOCK_ARG (NULL));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.swap_flash_devices,
		&host.flash_mgr, HOST_FLASH_MGR_SWAP_FAILED, MOCK_ARG (NULL), MOCK_ARG (NULL));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.swap_flash_devices,
		&host.flash_mgr, HOST_FLASH_MGR_SWAP_FAILED, MOCK_ARG (NULL), MOCK_ARG (NULL));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.swap_flash_devices,
		&host.flash_mgr, 0, MOCK_ARG (NULL), MOCK_ARG (NULL));

	status |= mock_expect (&host.observer.mock, host.observer.base.on_active_mode, &host.observer,
		0);

	status |= mock_expect (&host.pfm.mock, host.pfm.base.free_read_write_regions, &host.pfm, 0,
		MOCK_ARG_SAVED_ARG (0));

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_swap_error_pulse_reset (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;
	struct flash_region rw_region;
	struct pfm_read_write_regions rw_list;

	TEST_START;

	host_processor_dual_testing_init_pulse_reset (test, &host);

	rw_region.start_addr = 0x200;
	rw_region.length = 0x100;

	rw_list.regions = &rw_region;
	rw_list.count = 1;

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.validate_read_write_flash,
		&host.flash_mgr, 0, MOCK_ARG (&host.pfm), MOCK_ARG (&host.hash), MOCK_ARG (&host.rsa),
		MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&host.flash_mgr.mock, 3, &rw_list, sizeof (rw_list), -1);
	status |= mock_expect_save_arg (&host.flash_mgr.mock, 3, 0);
	status |= mock_expect_share_save_arg (&host.flash_mgr.mock, 0, &host.pfm.mock, 0);

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region, &host.filter,
		0, MOCK_ARG (1), MOCK_ARG (0x200), MOCK_ARG (0x300));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.swap_flash_devices,
		&host.flash_mgr, HOST_FLASH_MGR_SWAP_FAILED, MOCK_ARG (NULL), MOCK_ARG (NULL));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.swap_flash_devices,
		&host.flash_mgr, HOST_FLASH_MGR_SWAP_FAILED, MOCK_ARG (NULL), MOCK_ARG (NULL));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.swap_flash_devices,
		&host.flash_mgr, HOST_FLASH_MGR_SWAP_FAILED, MOCK_ARG (NULL), MOCK_ARG (NULL));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.swap_flash_devices,
		&host.flash_mgr, HOST_FLASH_MGR_SWAP_FAILED, MOCK_ARG (NULL), MOCK_ARG (NULL));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.swap_flash_devices,
		&host.flash_mgr, 0, MOCK_ARG (NULL), MOCK_ARG (NULL));

	status |= mock_expect (&host.observer.mock, host.observer.base.on_active_mode, &host.observer,
		0);

	status |= mock_expect (&host.pfm.mock, host.pfm.base.free_read_write_regions, &host.pfm, 0,
		MOCK_ARG_SAVED_ARG (0));

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_filter_error (CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;
	struct flash_region rw_region;
	struct pfm_read_write_regions rw_list;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	rw_region.start_addr = 0x200;
	rw_region.length = 0x100;

	rw_list.regions = &rw_region;
	rw_list.count = 1;

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.validate_read_write_flash,
		&host.flash_mgr, 0, MOCK_ARG (&host.pfm), MOCK_ARG (&host.hash), MOCK_ARG (&host.rsa),
		MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&host.flash_mgr.mock, 3, &rw_list, sizeof (rw_list), -1);
	status |= mock_expect_save_arg (&host.flash_mgr.mock, 3, 0);
	status |= mock_expect_share_save_arg (&host.flash_mgr.mock, 0, &host.pfm.mock, 0);

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, SPI_FILTER_CLEAR_DIRTY_FAILED);
	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, SPI_FILTER_CLEAR_DIRTY_FAILED);
	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, SPI_FILTER_CLEAR_DIRTY_FAILED);
	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, SPI_FILTER_CLEAR_DIRTY_FAILED);
	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region, &host.filter,
		0, MOCK_ARG (1), MOCK_ARG (0x200), MOCK_ARG (0x300));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.swap_flash_devices,
		&host.flash_mgr, 0, MOCK_ARG (NULL), MOCK_ARG (NULL));

	status |= mock_expect (&host.observer.mock, host.observer.base.on_active_mode, &host.observer,
		0);

	status |= mock_expect (&host.pfm.mock, host.pfm.base.free_read_write_regions, &host.pfm, 0,
		MOCK_ARG_SAVED_ARG (0));

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_filter_error_pulse_reset (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;
	struct flash_region rw_region;
	struct pfm_read_write_regions rw_list;

	TEST_START;

	host_processor_dual_testing_init_pulse_reset (test, &host);

	rw_region.start_addr = 0x200;
	rw_region.length = 0x100;

	rw_list.regions = &rw_region;
	rw_list.count = 1;

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.validate_read_write_flash,
		&host.flash_mgr, 0, MOCK_ARG (&host.pfm), MOCK_ARG (&host.hash), MOCK_ARG (&host.rsa),
		MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&host.flash_mgr.mock, 3, &rw_list, sizeof (rw_list), -1);
	status |= mock_expect_save_arg (&host.flash_mgr.mock, 3, 0);
	status |= mock_expect_share_save_arg (&host.flash_mgr.mock, 0, &host.pfm.mock, 0);

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, SPI_FILTER_CLEAR_DIRTY_FAILED);
	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, SPI_FILTER_CLEAR_DIRTY_FAILED);
	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, SPI_FILTER_CLEAR_DIRTY_FAILED);
	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, SPI_FILTER_CLEAR_DIRTY_FAILED);
	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region, &host.filter,
		0, MOCK_ARG (1), MOCK_ARG (0x200), MOCK_ARG (0x300));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.swap_flash_devices,
		&host.flash_mgr, 0, MOCK_ARG (NULL), MOCK_ARG (NULL));

	status |= mock_expect (&host.observer.mock, host.observer.base.on_active_mode, &host.observer,
		0);

	status |= mock_expect (&host.pfm.mock, host.pfm.base.free_read_write_regions, &host.pfm, 0,
		MOCK_ARG_SAVED_ARG (0));

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_host_access_error (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;
	struct flash_region rw_region;
	struct pfm_read_write_regions rw_list;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	rw_region.start_addr = 0x200;
	rw_region.length = 0x100;

	rw_list.regions = &rw_region;
	rw_list.count = 1;

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.validate_read_write_flash,
		&host.flash_mgr, 0, MOCK_ARG (&host.pfm), MOCK_ARG (&host.hash), MOCK_ARG (&host.rsa),
		MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&host.flash_mgr.mock, 3, &rw_list, sizeof (rw_list), -1);
	status |= mock_expect_save_arg (&host.flash_mgr.mock, 3, 0);
	status |= mock_expect_share_save_arg (&host.flash_mgr.mock, 0, &host.pfm.mock, 0);

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region, &host.filter,
		0, MOCK_ARG (1), MOCK_ARG (0x200), MOCK_ARG (0x300));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.swap_flash_devices,
		&host.flash_mgr, 0, MOCK_ARG (NULL), MOCK_ARG (NULL));

	status |= mock_expect (&host.observer.mock, host.observer.base.on_active_mode, &host.observer,
		0);

	status |= mock_expect (&host.pfm.mock, host.pfm.base.free_read_write_regions, &host.pfm, 0,
		MOCK_ARG_SAVED_ARG (0));

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_host_access_error_pulse_reset (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;
	struct flash_region rw_region;
	struct pfm_read_write_regions rw_list;

	TEST_START;

	host_processor_dual_testing_init_pulse_reset (test, &host);

	rw_region.start_addr = 0x200;
	rw_region.length = 0x100;

	rw_list.regions = &rw_region;
	rw_list.count = 1;

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.validate_read_write_flash,
		&host.flash_mgr, 0, MOCK_ARG (&host.pfm), MOCK_ARG (&host.hash), MOCK_ARG (&host.rsa),
		MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&host.flash_mgr.mock, 3, &rw_list, sizeof (rw_list), -1);
	status |= mock_expect_save_arg (&host.flash_mgr.mock, 3, 0);
	status |= mock_expect_share_save_arg (&host.flash_mgr.mock, 0, &host.pfm.mock, 0);

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region, &host.filter,
		0, MOCK_ARG (1), MOCK_ARG (0x200), MOCK_ARG (0x300));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.swap_flash_devices,
		&host.flash_mgr, 0, MOCK_ARG (NULL), MOCK_ARG (NULL));

	status |= mock_expect (&host.observer.mock, host.observer.base.on_active_mode, &host.observer,
		0);

	status |= mock_expect (&host.pfm.mock, host.pfm.base.free_read_write_regions, &host.pfm, 0,
		MOCK_ARG_SAVED_ARG (0));

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_host_access_error_after_validation_fail (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.validate_read_write_flash,
		&host.flash_mgr, RSA_ENGINE_BAD_SIGNATURE, MOCK_ARG (&host.pfm), MOCK_ARG (&host.hash),
		MOCK_ARG (&host.rsa),
		MOCK_ARG_NOT_NULL);

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, RSA_ENGINE_BAD_SIGNATURE, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_host_access_error_after_validation_fail_pulse_reset (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init_pulse_reset (test, &host);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.validate_read_write_flash,
		&host.flash_mgr, RSA_ENGINE_BAD_SIGNATURE, MOCK_ARG (&host.pfm), MOCK_ARG (&host.hash),
		MOCK_ARG (&host.rsa),
		MOCK_ARG_NOT_NULL);

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, HOST_FLASH_MGR_HOST_ACCESS_FAILED, MOCK_ARG (&host.control));
	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, RSA_ENGINE_BAD_SIGNATURE, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, false, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_bypass_filter_error (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	host_state_manager_set_bypass_mode (&host.host_state, true);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, SPI_FILTER_CLEAR_RW_FAILED);
	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, SPI_FILTER_CLEAR_RW_FAILED);
	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, SPI_FILTER_CLEAR_RW_FAILED);
	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, SPI_FILTER_CLEAR_RW_FAILED);
	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region,
		&host.filter, 0, MOCK_ARG (1), MOCK_ARG (0), MOCK_ARG (0xffff0000));

	status |= mock_expect (&host.filter.mock, host.filter.base.set_ro_cs, &host.filter, 0,
		MOCK_ARG (SPI_FILTER_CS_0));

	status |= mock_expect (&host.observer.mock, host.observer.base.on_bypass_mode, &host.observer,
		0);

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_get_read_only_flash (&host.host_state);
	CuAssertIntEquals (test, SPI_FILTER_CS_1, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}

static void host_processor_dual_test_flash_rollback_active_pfm_not_dirty_bypass_cs_error (
	CuTest *test)
{
	struct host_processor_dual_testing host;
	int status;

	TEST_START;

	host_processor_dual_testing_init (test, &host);

	host_state_manager_set_bypass_mode (&host.host_state, true);

	status = mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.get_active_pfm, &host.pfm_mgr,
		(intptr_t) &host.pfm);

	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (true));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_rot_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region,
		&host.filter, 0, MOCK_ARG (1), MOCK_ARG (0), MOCK_ARG (0xffff0000));
	status |= mock_expect (&host.filter.mock, host.filter.base.set_ro_cs, &host.filter,
		SPI_FILTER_SET_RO_FAILED, MOCK_ARG (SPI_FILTER_CS_0));

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region,
		&host.filter, 0, MOCK_ARG (1), MOCK_ARG (0), MOCK_ARG (0xffff0000));
	status |= mock_expect (&host.filter.mock, host.filter.base.set_ro_cs, &host.filter,
		SPI_FILTER_SET_RO_FAILED, MOCK_ARG (SPI_FILTER_CS_0));

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region,
		&host.filter, 0, MOCK_ARG (1), MOCK_ARG (0), MOCK_ARG (0xffff0000));
	status |= mock_expect (&host.filter.mock, host.filter.base.set_ro_cs, &host.filter,
		SPI_FILTER_SET_RO_FAILED, MOCK_ARG (SPI_FILTER_CS_0));

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region,
		&host.filter, 0, MOCK_ARG (1), MOCK_ARG (0), MOCK_ARG (0xffff0000));
	status |= mock_expect (&host.filter.mock, host.filter.base.set_ro_cs, &host.filter,
		SPI_FILTER_SET_RO_FAILED, MOCK_ARG (SPI_FILTER_CS_0));

	status |= mock_expect (&host.filter.mock, host.filter.base.clear_filter_rw_regions,
		&host.filter, 0);
	status |= mock_expect (&host.filter.mock, host.filter.base.set_filter_rw_region,
		&host.filter, 0, MOCK_ARG (1), MOCK_ARG (0), MOCK_ARG (0xffff0000));
	status |= mock_expect (&host.filter.mock, host.filter.base.set_ro_cs, &host.filter, 0,
		MOCK_ARG (SPI_FILTER_CS_0));

	status |= mock_expect (&host.observer.mock, host.observer.base.on_bypass_mode, &host.observer,
		0);

	status |= mock_expect (&host.pfm_mgr.mock, host.pfm_mgr.base.free_pfm, &host.pfm_mgr, 0,
		MOCK_ARG (&host.pfm));

	status |= mock_expect (&host.flash_mgr.mock, host.flash_mgr.base.set_flash_for_host_access,
		&host.flash_mgr, 0, MOCK_ARG (&host.control));
	status |= mock_expect (&host.control.mock, host.control.base.hold_processor_in_reset,
		&host.control, 0, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host.test.base.flash_rollback (&host.test.base, &host.hash.base, &host.rsa.base,
		false, false);
	CuAssertIntEquals (test, 0, status);

	status = host_state_manager_is_pfm_dirty (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_is_bypass_mode (&host.host_state);
	CuAssertIntEquals (test, true, status);

	status = host_state_manager_get_read_only_flash (&host.host_state);
	CuAssertIntEquals (test, SPI_FILTER_CS_1, status);

	host_processor_dual_testing_validate_and_release (test, &host);
}


CuSuite* get_host_processor_dual_flash_rollback_suite ()
{
	CuSuite *suite = CuSuiteNew ();

	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_no_pfm);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_no_pfm_bypass);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_no_pfm_checked);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_no_pfm_checked_bypass);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_no_pfm_dirty);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_no_pfm_dirty_bypass);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_no_pfm_dirty_checked);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_no_pfm_dirty_checked_bypass);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_no_pfm_pulse_reset);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_no_pfm_disable_bypass);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_no_pfm_no_reset);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_no_pfm_no_reset_pulse_reset);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_no_pfm_unsupported_flash);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_active_pfm_not_dirty);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_active_pfm_not_dirty_bypass);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_active_pfm_not_dirty_bypass_cs1);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_not_dirty_bypass_no_observer);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_active_pfm_not_dirty_checked);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_not_dirty_checked_bypass);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_not_dirty_pulse_reset);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_not_dirty_bypass_disable_bypass);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_active_pfm_not_dirty_no_reset);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_not_dirty_no_reset_pulse_reset);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_not_dirty_unsupported_flash);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_active_pfm_dirty);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_active_pfm_dirty_bypass);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_active_pfm_dirty_checked);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_active_pfm_dirty_checked_bypass);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_dirty_bypass_disable_bypass);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_dirty_bypass_no_reset);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_dirty_bypass_no_reset_pulse_reset);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_dirty_unsupported_flash);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_not_dirty_validation_fail);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_not_dirty_hash_validation_fail);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_not_dirty_unknown_version);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_active_pfm_not_dirty_blank_fail);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_validation_fail_pulse_reset);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_hash_validation_fail_pulse_reset);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_unknown_version_pulse_reset);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_blank_fail_pulse_reset);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_null);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_no_pfm_rot_access_error);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_no_pfm_rot_access_error_pulse_reset);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_no_pfm_rot_access_error_host_access_error);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_no_pfm_erase_error);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_no_pfm_erase_error_pulse_reset);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_no_pfm_copy_error);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_no_pfm_copy_error_pulse_reset);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_no_pfm_host_access_error);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_no_pfm_host_access_error_pulse_reset);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_not_dirty_rot_access_error);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_not_dirty_rot_access_error_pulse_reset);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_not_dirty_rot_access_error_host_access_error);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_not_dirty_validation_error);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_not_dirty_validation_error_pulse_reset);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_active_pfm_not_dirty_swap_error);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_not_dirty_swap_error_pulse_reset);
	SUITE_ADD_TEST (suite, host_processor_dual_test_flash_rollback_active_pfm_not_dirty_filter_error);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_not_dirty_filter_error_pulse_reset);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_not_dirty_host_access_error);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_not_dirty_host_access_error_pulse_reset);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_not_dirty_host_access_error_after_validation_fail);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_not_dirty_host_access_error_after_validation_fail_pulse_reset);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_not_dirty_bypass_filter_error);
	SUITE_ADD_TEST (suite,
		host_processor_dual_test_flash_rollback_active_pfm_not_dirty_bypass_cs_error);

	return suite;
}
