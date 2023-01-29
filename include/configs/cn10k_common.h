/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#ifndef __CN10K_A_H__
#define __CN10K_A_H__

#define CONFIG_SYS_SDRAM_BASE		CONFIG_TEXT_BASE

/** Extra environment settings */
#define CONFIG_EXTRA_ENV_SETTINGS	\
					"loadaddr=020080000\0"	\
					"ethrotate=yes\0"	\
					"autoload=0\0"

#undef CONFIG_SYS_PROMPT
#define CONFIG_SYS_PROMPT		env_get("prompt")

#endif /* __CN10K_A_H__ */
