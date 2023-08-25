// SPDX-License-Identifier: BSD-2-Clause-Patent
/*
 *  https://spdx.org/licenses
 *  Copyright (c) 2023 Marvell
 *
 *  EFI I2C bus protocol
 */

#include <common.h>
#include <efi_loader.h>
#include <linux/sizes.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <dm/uclass-internal.h>
#include <malloc.h>
#include <i2c.h>

const efi_guid_t efi_guid_i2c_protocol = EFI_I2C_PROTOCOL_GUID;

struct efi_i2c_bus_protocol_obj {
	struct efi_object header;
	struct efi_i2c_protocol efi_i2c_proto;
	struct udevice *i2c_ctrl;
};

efi_status_t EFIAPI i2c_probe_device(const struct efi_i2c_protocol *this,
				     u8 dev_addr)
{
	efi_status_t efi_stat;
	int rc;
	struct udevice *i2c_dev;
	struct efi_i2c_bus_protocol_obj *parent;

	parent = container_of(this, struct efi_i2c_bus_protocol_obj,
			      efi_i2c_proto);

	EFI_ENTRY("%d", dev_addr);

	if (!this || !parent)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	rc = dm_i2c_probe(parent->i2c_ctrl, dev_addr, 0, &i2c_dev);
	if (!rc)
		efi_stat = EFI_SUCCESS;
	else
		efi_stat = EFI_DEVICE_ERROR;

	return EFI_EXIT(efi_stat);
}

efi_status_t EFIAPI i2c_read_device(const struct efi_i2c_protocol *this,
				    u8 dev_addr, u32 reg_addr, u32 addr_size,
				    u32 length, void *buffer)
{
	efi_status_t efi_stat;
	int rc;
	struct efi_i2c_bus_protocol_obj *parent;
	struct dm_i2c_ops *ops;
	struct i2c_msg msg;

	parent = container_of(this, struct efi_i2c_bus_protocol_obj,
			      efi_i2c_proto);

	EFI_ENTRY("%d %d", dev_addr, reg_addr);

	if (!this)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	ops = i2c_get_ops(parent->i2c_ctrl);
	if (!ops->xfer)
		return EFI_EXIT(EFI_PROTOCOL_ERROR);

	msg.addr = dev_addr;
	msg.buf = buffer;
	msg.len = length;
	msg.flags |= I2C_M_RD;
	rc = ops->xfer(parent->i2c_ctrl, &msg, 1);
	if (!rc)
		efi_stat = EFI_SUCCESS;
	else
		efi_stat = EFI_DEVICE_ERROR;

	return EFI_EXIT(efi_stat);
}

efi_status_t EFIAPI i2c_write_device(const struct efi_i2c_protocol *this,
				     u8 dev_addr, u32 reg_addr, u32 addr_size,
				     u32 length, void *buffer)
{
	efi_status_t efi_stat;
	int rc;
	struct efi_i2c_bus_protocol_obj *parent;
	struct dm_i2c_ops *ops;
	struct i2c_msg msg;

	parent = container_of(this, struct efi_i2c_bus_protocol_obj,
			      efi_i2c_proto);

	EFI_ENTRY("%d %d", dev_addr, reg_addr);

	if (!this)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	ops = i2c_get_ops(parent->i2c_ctrl);
	if (!ops->xfer)
		return EFI_EXIT(EFI_PROTOCOL_ERROR);

	msg.addr = dev_addr;
	msg.buf = buffer;
	msg.len = length;
	msg.flags = 0;
	rc = ops->xfer(parent->i2c_ctrl, &msg, 1);
	if (!rc)
		efi_stat = EFI_SUCCESS;
	else
		efi_stat = EFI_DEVICE_ERROR;

	return EFI_EXIT(efi_stat);
}

efi_status_t efi_i2c_protocol_register(void)
{
	efi_status_t r;
	struct udevice *i2c_bus;
	struct efi_i2c_bus_protocol_obj *i2c_prot_obj;

	uclass_first_device(UCLASS_I2C, &i2c_bus);
	while (i2c_bus) {
		//Create I2C bus object
		i2c_prot_obj = calloc(1, sizeof(*i2c_prot_obj));
		if (!i2c_prot_obj)
			goto err;

		/* Hook i2c up to the device list */
		efi_add_handle(&i2c_prot_obj->header);

		i2c_prot_obj->efi_i2c_proto.i2c_probe_device = i2c_probe_device;
		i2c_prot_obj->efi_i2c_proto.i2c_read_device = i2c_read_device;
		i2c_prot_obj->efi_i2c_proto.i2c_write_device = i2c_write_device;
		r = efi_add_protocol(&i2c_prot_obj->header, &efi_guid_i2c_protocol,
				     &i2c_prot_obj->efi_i2c_proto);
		if (r != EFI_SUCCESS)
			goto err;

		i2c_prot_obj->i2c_ctrl = i2c_bus;
		uclass_next_device(&i2c_bus);
	}
	return EFI_SUCCESS;
err:
	free(i2c_prot_obj);
	return EFI_PROTOCOL_ERROR;
}
