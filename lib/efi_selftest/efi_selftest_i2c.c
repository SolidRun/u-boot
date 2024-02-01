// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Marvell
 *
 * EFI I2C protocol self test
 *
 */

#include <efi_selftest.h>

#define FIRST_I2C_ADDR		01
#define LAST_I2C_ADDR		127

static struct efi_boot_services *boottime;
const efi_guid_t efi_guid_i2c = EFI_I2C_PROTOCOL_GUID;
static struct efi_i2c_protocol *efi_i2c;

/*
 * Setup unit test.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * @return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	boottime = systable->boottime;
	return EFI_ST_SUCCESS;
}

/*
 * Tear down unit test.
 *
 * @return:	EFI_ST_SUCCESS for success
 */
static int teardown(void)
{
	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * @return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	efi_status_t ret;
	u8 dev_addr, data;
	efi_handle_t *buffer;
	size_t count;

	if (!boottime)
		return EFI_ST_FAILURE;

	ret = boottime->locate_handle_buffer(BY_PROTOCOL, &efi_guid_i2c, NULL, &count, &buffer);
	if (ret)
		return EFI_ST_FAILURE;

	while (count--) {
		ret = boottime->handle_protocol(buffer[count], &efi_guid_i2c, (void **)&efi_i2c);
		if (ret || !efi_i2c)
			continue;

		// Scan all I2C devices
		for (dev_addr = FIRST_I2C_ADDR; dev_addr <= LAST_I2C_ADDR; dev_addr++) {
			// Probe
			if (efi_i2c->i2c_probe_device)
				ret = efi_i2c->i2c_probe_device(efi_i2c, dev_addr);
			else
				break;
			if (!ret) {
				efi_st_printf("I2C Bus:%d Device:%d\n", count, dev_addr);
				ret = efi_i2c->i2c_read_device(efi_i2c, dev_addr, 0, 1, 1, &data);
				if (!ret)
					efi_st_printf("Byte@0 : %d\n", data);
			}
		}
	}
	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(swcfg) = {
	.name = "i2c ",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.teardown = teardown,
};
