// SPDX-License-Identifier: BSD-2-Clause-Patent
/*
 *  https://spdx.org/licenses
 *  Copyright (c) 2023 Marvell
 *
 *  EFI GPIO protocol
 */

#include <common.h>
#include <efi_loader.h>
#include <linux/sizes.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <asm-generic/gpio.h>

const efi_guid_t efi_guid_gpio_protocol = EFI_GPIO_PROTOCOL_GUID;
efi_handle_t gpio_handle;
struct udevice *gpio_dev;
// HRM - Total 76 GPIO pins
#define MAX_GPIO_PIN	75

efi_status_t EFIAPI efi_gpio_get(const struct efi_gpio_protocol *this,
				 u32 gpio_num, u8 *value)
{
	struct gpio_desc desc;
	efi_status_t efi_stat;
	int	rc;

	EFI_ENTRY("%d", gpio_num);

	if (!this || gpio_num > MAX_GPIO_PIN || !value)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	efi_stat = EFI_SUCCESS;
	desc.dev = gpio_dev;
	desc.offset = gpio_num;
	rc = dm_gpio_request(&desc, "efi-gpio");
	if (rc) {
		debug("requesting gpio: %d failed\n", gpio_num);
		return EFI_EXIT(EFI_OUT_OF_RESOURCES);
	}

	*value = dm_gpio_get_value(&desc);

	rc = dm_gpio_free(gpio_dev, &desc);
	if (rc) {
		debug("freeing gpio: %d failed\n", gpio_num);
		efi_stat = EFI_DEVICE_ERROR;
	}

	return EFI_EXIT(efi_stat);
}

efi_status_t EFIAPI efi_gpio_set(const struct efi_gpio_protocol *this,
				 u32 gpio_num, u8 value)
{
	int rc;
	struct gpio_desc desc;
	efi_status_t efi_stat;

	EFI_ENTRY("%d %d", gpio_num, value);

	if (!this || gpio_num > MAX_GPIO_PIN)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	efi_stat = EFI_SUCCESS;
	desc.dev = gpio_dev;
	desc.offset = gpio_num;
	rc = dm_gpio_request(&desc, "efi-gpio");
	if (rc) {
		debug("requesting gpio: %d failed\n", gpio_num);
		return EFI_EXIT(EFI_OUT_OF_RESOURCES);
	}

	rc = dm_gpio_set_value(&desc, value);
	if (rc) {
		debug("setting gpio: %d failed\n", gpio_num);
		efi_stat = EFI_DEVICE_ERROR;
	}

	rc = dm_gpio_free(gpio_dev, &desc);
	if (rc) {
		debug("freeing gpio: %d failed\n", gpio_num);
		efi_stat = EFI_DEVICE_ERROR;
	}

	return EFI_EXIT(efi_stat);
}

efi_status_t EFIAPI efi_gpio_get_mode(const struct efi_gpio_protocol *this,
				      u32 gpio_num, u32 *mode)
{
	EFI_ENTRY("%d", gpio_num);

	if (!this || gpio_num > MAX_GPIO_PIN || !mode)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

efi_status_t EFIAPI efi_gpio_set_pull(const struct efi_gpio_protocol *this,
				      u32 gpio_num, u8 pull)
{
	EFI_ENTRY("%d %d", gpio_num, pull);

	if (!this || gpio_num > MAX_GPIO_PIN)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

struct efi_gpio_protocol gpio_proto = {
	.gpio_get = efi_gpio_get,
	.gpio_set = efi_gpio_set,
	.gpio_get_mode = efi_gpio_get_mode,
	.gpio_set_pull = efi_gpio_set_pull,
};

efi_status_t efi_gpio_protocol_register(void)
{
	efi_status_t r;

	uclass_first_device(UCLASS_GPIO, &gpio_dev);
	if (!gpio_dev) {
		debug("No GPIO device detected\n");
		return EFI_DEVICE_ERROR;
	}

	/* Create handles */
	r = efi_create_handle(&gpio_handle);
	if (r != EFI_SUCCESS) {
		debug("%s:%d ERROR: Out of memory\n", __func__, __LINE__);
		return EFI_OUT_OF_RESOURCES;
	}

	/* Register protocol handle */
	r = efi_add_protocol(gpio_handle,
			     &efi_guid_gpio_protocol, &gpio_proto);
	if (r != EFI_SUCCESS) {
		debug("%s ERROR: Failure to add protocol\n", __func__);
		return r;
	}

	return EFI_SUCCESS;
}
