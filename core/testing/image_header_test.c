// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "testing.h"
#include "mock/flash_mock.h"
#include "common/image_header.h"
#include "image_header_testing.h"
#include "recovery/recovery_image_header.h"
#include "recovery_image_header_testing.h"


static const char *SUITE = "image_header";


/*******************
 * Test cases
 *******************/

static void image_header_test_get_length (CuTest *test)
{
	struct flash_mock flash;
	struct recovery_image_header header;
	int status;

	TEST_START;

	status = flash_mock_init (&flash);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&flash.mock, flash.base.read, &flash, 0, MOCK_ARG (0x10000),
		MOCK_ARG_NOT_NULL, MOCK_ARG (IMAGE_HEADER_BASE_LEN));
	status |= mock_expect_output (&flash.mock, 1, RECOVERY_IMAGE_HEADER_FORMAT_0,
		RECOVERY_IMAGE_HEADER_FORMAT_0_TOTAL_LEN, 2);
	CuAssertIntEquals (test, 0, status);

	status = image_header_init (&header.base, &flash.base, 0x10000, RECOVERY_IMAGE_HEADER_MARKER,
		RECOVERY_IMAGE_HEADER_MAX_LENGTH);
	CuAssertIntEquals (test, 0, status);

	status = mock_validate (&flash.mock);
	CuAssertIntEquals (test, 0, status);

	status = image_header_get_length (&header.base);
	CuAssertIntEquals (test, RECOVERY_IMAGE_HEADER_FORMAT_0_TOTAL_LEN, status);

	status = flash_mock_validate_and_release (&flash);
	CuAssertIntEquals (test, 0, status);

	image_header_release (&header.base);
}

static void image_header_test_get_length_null (CuTest *test)
{
	int status;

	TEST_START;

	status = image_header_get_length (NULL);
	CuAssertIntEquals (test, 0, status);
}


CuSuite* get_image_header_suite ()
{
	CuSuite *suite = CuSuiteNew ();

	SUITE_ADD_TEST (suite, image_header_test_get_length);
	SUITE_ADD_TEST (suite, image_header_test_get_length_null);

	return suite;
}
