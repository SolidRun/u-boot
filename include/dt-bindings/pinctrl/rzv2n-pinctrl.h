/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Defines macros and constants for Renesas RZ/V2N pin controller pin
 * muxing functions.
 */
#ifndef __DT_BINDINGS_RZV2N_PINCTRL_H
#define __DT_BINDINGS_RZV2N_PINCTRL_H

#define RZV2N_PINS_PER_PORT	8

/*
 * Store the pin index from its port and position number in bits[11-0].
 * And store its peripheral function mode identifier in 3 bits [14-12]
 */
#define RZV2N_PINMUX(port, pos, func)	\
	(((port) * RZV2N_PINS_PER_PORT + (pos)) | ((func) << 12))

#endif /* __DT_BINDINGS_RZV2N_PINCTRL_H */
