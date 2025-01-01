// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2021 Alvaro Karsz <alvaro.karsz@solid-run.com>
 * Copyright 2025 Josua Mayer <josua@solid-run.com>
 */

#include <button.h>
#include <command.h>
#include <linux/err.h>

static struct udevice *button_power, *button_charge;

static int supercap_get_status(bool *powered, bool *charged)
{
	int ret;

	/* get power fail button */
	if (IS_ERR_OR_NULL(button_power)) {
		ret = button_get_by_label("Power Fail", &button_power);
		if (ret) {
			printf("Failed to get supercap \"Power Fail\" button: %d\n", ret);
			return ret;
		}
	}

	/* get full charge button */
	if (IS_ERR_OR_NULL(button_charge)) {
		ret = button_get_by_label("Low Charge", &button_charge);
		if (ret) {
			printf("Failed to get supercap \"Full Charge\" button: %d\n", ret);
			return ret;
		}
	}

	/* query power fail button */
	ret = button_get_state(button_power);
	if (ret < 0) {
		printf("Failed to get supercap \"Power Fail\" button state: %d\n", ret);
		return CMD_RET_FAILURE;
	}
	*powered = (ret == BUTTON_OFF);

	/* query full charge button */
	ret = button_get_state(button_charge);
	if (ret < 0) {
		printf("Failed to get supercap \"Full Charge\" button state: %d\n", ret);
		return ret;
	}
	*charged = (ret == BUTTON_OFF);

	return 0;
}

/*
 * Function that can be called from U-boot
 * This function will check if power is connected.
 * If power is not connected, it will keep checking forever.
 *
 * function receives 1 argument <mode>: this function has 2 modes:
 * mode = 1: boot if power is connected.
 * mode = 2: boot if power is connected and capacitor charge is over 90%.
 */
static int check_power_connection(struct cmd_tbl *cmdtp, int flag, int argc,
				  char *const argv[])
{
	int ret;
	char mode; /* function mode, get from argv */
	bool powered, charged;

	/* validate number of arguments */
	if (argc == 1) {
		/* no arguments, set default mode - 0 */
		mode = '0';
	} else if (argc == 2) {
		/* get mode from argv */
		mode = *argv[1];
	} else {
		/* invalid number of arguments */
		return cmd_usage(cmdtp);
	}

	/* check mode */
	switch (mode) {
	case '0':
		printf("Power check - show status\n");

		if (supercap_get_status(&powered, &charged))
			return CMD_RET_FAILURE;

		if (powered)
			puts("Powered, ");
		else
			puts("Not powered, ");
		if (charged)
			puts("Charged.\n");
		else
			puts("Not charged.\n");

		return CMD_RET_SUCCESS;
	case '1':
		printf("Power check - wait till power is connected\n");

		do {
			ret = supercap_get_status(&powered, &charged);
			if (ret)
				return CMD_RET_FAILURE;
		} while (!powered);

		return CMD_RET_SUCCESS;
	case '2':
		printf("Power check - wait till power is connected and capacitor charge is 90%%\n");

		do {
			ret = supercap_get_status(&powered, &charged);
			if (ret)
				return CMD_RET_FAILURE;
		} while (!powered || !charged);

		return CMD_RET_SUCCESS;
	default: /*ilegal mode, ignore function and boot*/
		printf("Power check - illegal mode\n");
		return cmd_usage(cmdtp);
	}
}

U_BOOT_CMD(
		check_power_connection, 2, 0, check_power_connection,
		"Check if power is connected\n"
		"	check_power_connection <mode>\n"
		"	mode = 0: show status\n"
		"	mode = 1: wait till power is connected\n"
		"	mode = 2: wait till power is connected and capacitor charge is over 90%\n",
		" <mode>\n"
);
