/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */


#ifndef OCTEONTX_XCV_H_
#define OCTEONTX_XCV_H_

#define CAVM_XCVX_RESET		0x87E0DB000000ULL
#define CAVM_XCVX_DLL_CTL	0x87E0DB000010ULL
#define CAVM_XCVX_CTL		0x87E0DB000030ULL
#define CAVM_XCVX_BATCH_CRD_RET	0x87E0DB000100ULL

/**
 * Register (RSL) xcv#_dll_ctl
 *
 * XCV DLL Controller Register
 * The RGMII timing specification requires that devices transmit clock and
 * data synchronously. The specification requires external sources (namely
 * the PC board trace routes) to introduce the appropriate 1.5 to 2.0 ns of
 * delay.
 *
 * To eliminate the need for the PC board delays, the RGMII interface has optional
 * on-board DLLs for both transmit and receive. For correct operation, at most one
 * of the transmitter, board, or receiver involved in an RGMII link should
 * introduce delay. By default/reset, the RGMII receivers delay the received clock,
 * and the RGMII transmitters do not delay the transmitted clock. Whether this
 * default works as-is with a given link partner depends on the behavior of the
 * link partner and the PC board.
 *
 * These are the possible modes of RGMII receive operation:
 *
 * * XCV()_DLL_CTL[CLKRX_BYP] = 0 (reset value) - The RGMII
 * receive interface introduces clock delay using its internal DLL.
 * This mode is appropriate if neither the remote
 * transmitter nor the PC board delays the clock.
 *
 * * XCV()_DLL_CTL[CLKRX_BYP] = 1, [CLKRX_SET] = 0x0 - The
 * RGMII receive interface introduces no clock delay. This mode
 * is appropriate if either the remote transmitter or the PC board
 * delays the clock.
 *
 * These are the possible modes of RGMII transmit operation:
 *
 * * XCV()_DLL_CTL[CLKTX_BYP] = 1, [CLKTX_SET] = 0x0 (reset value) -
 * The RGMII transmit interface introduces no clock
 * delay. This mode is appropriate is either the remote receiver
 * or the PC board delays the clock.
 *
 * * XCV()_DLL_CTL[CLKTX_BYP] = 0 - The RGMII transmit
 * interface introduces clock delay using its internal DLL.
 * This mode is appropriate if neither the remote receiver
 * nor the PC board delays the clock.
 */
union cavm_xcvx_dll_ctl
{
    uint64_t u;
    struct cavm_xcvx_dll_ctl_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_32_63        : 32;
        uint64_t lock                  : 1;  /**< [ 31: 31](RO/H) DLL is locked. */
        uint64_t clk_set               : 7;  /**< [ 30: 24](RO/H) The clock delay as determined by the on-board hardware DLL. */
        uint64_t clkrx_byp             : 1;  /**< [ 23: 23](R/W) Bypass the RX clock delay setting.
                                                                 Skews RXC from RXD, RXCTL.
                                                                 By default, hardware internally shifts the RXC clock
                                                                 to sample RXD,RXCTL assuming clock and data and
                                                                 sourced synchronously from the link partner. */
        uint64_t clkrx_set             : 7;  /**< [ 22: 16](R/W) RX clock delay setting to use in bypass mode.
                                                                 Skews RXC from RXD. */
        uint64_t clktx_byp             : 1;  /**< [ 15: 15](R/W) Bypass the TX clock delay setting.
                                                                 Skews TXC from TXD, TXCTL.
                                                                 By default, clock and data and sourced
                                                                 synchronously. */
        uint64_t clktx_set             : 7;  /**< [ 14:  8](R/W) TX clock delay setting to use in bypass mode.
                                                                 Skews TXC from TXD. */
        uint64_t reserved_2_7          : 6;
        uint64_t refclk_sel            : 2;  /**< [  1:  0](R/W) Select the reference clock to use.  Normal RGMII specification requires a 125MHz
                                                                 oscillator.
                                                                 To reduce system cost, a 500MHz coprocessor clock can be divided down and remove the
                                                                 requirements for the external oscillator. Additionally, in some well defined systems, the
                                                                 link partner may be able to source the RXC. The RGMII would operate correctly in 1000Mbs
                                                                 mode only.
                                                                 0x0 = RGMII REFCLK.
                                                                 0x1 = RGMII RXC (1000Mbs only).
                                                                 0x2 = Divided coprocessor clock.
                                                                 0x3 = Reserved.

                                                                 Internal:
                                                                 Some programming magic could allow for 10/100 operation if
                                                                 critical, use encoding 0x1 but some programming restrictions would apply. */
#else /* Word 0 - Little Endian */
        uint64_t refclk_sel            : 2;  /**< [  1:  0](R/W) Select the reference clock to use.  Normal RGMII specification requires a 125MHz
                                                                 oscillator.
                                                                 To reduce system cost, a 500MHz coprocessor clock can be divided down and remove the
                                                                 requirements for the external oscillator. Additionally, in some well defined systems, the
                                                                 link partner may be able to source the RXC. The RGMII would operate correctly in 1000Mbs
                                                                 mode only.
                                                                 0x0 = RGMII REFCLK.
                                                                 0x1 = RGMII RXC (1000Mbs only).
                                                                 0x2 = Divided coprocessor clock.
                                                                 0x3 = Reserved.

                                                                 Internal:
                                                                 Some programming magic could allow for 10/100 operation if
                                                                 critical, use encoding 0x1 but some programming restrictions would apply. */
        uint64_t reserved_2_7          : 6;
        uint64_t clktx_set             : 7;  /**< [ 14:  8](R/W) TX clock delay setting to use in bypass mode.
                                                                 Skews TXC from TXD. */
        uint64_t clktx_byp             : 1;  /**< [ 15: 15](R/W) Bypass the TX clock delay setting.
                                                                 Skews TXC from TXD, TXCTL.
                                                                 By default, clock and data and sourced
                                                                 synchronously. */
        uint64_t clkrx_set             : 7;  /**< [ 22: 16](R/W) RX clock delay setting to use in bypass mode.
                                                                 Skews RXC from RXD. */
        uint64_t clkrx_byp             : 1;  /**< [ 23: 23](R/W) Bypass the RX clock delay setting.
                                                                 Skews RXC from RXD, RXCTL.
                                                                 By default, hardware internally shifts the RXC clock
                                                                 to sample RXD,RXCTL assuming clock and data and
                                                                 sourced synchronously from the link partner. */
        uint64_t clk_set               : 7;  /**< [ 30: 24](RO/H) The clock delay as determined by the on-board hardware DLL. */
        uint64_t lock                  : 1;  /**< [ 31: 31](RO/H) DLL is locked. */
        uint64_t reserved_32_63        : 32;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_xcvx_dll_ctl_s cn; */
};


/**
 * Register (RSL) xcv#_reset
 *
 * XCV Reset Registers
 * This register controls reset.
 */
union cavm_xcvx_reset
{
    uint64_t u;
    struct cavm_xcvx_reset_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t enable                : 1;  /**< [ 63: 63](R/W) Port enable. */
        uint64_t reserved_16_62        : 47;
        uint64_t clkrst                : 1;  /**< [ 15: 15](R/W) DLL CLK reset. [CLKRST] must be set if DLL bypass mode
                                                                 XCV_DLL_CTL[CLKRX_BYP,CLKTX_BYP] is used. */
        uint64_t reserved_12_14        : 3;
        uint64_t dllrst                : 1;  /**< [ 11: 11](R/W) DLL reset. */
        uint64_t reserved_8_10         : 3;
        uint64_t comp                  : 1;  /**< [  7:  7](R/W) Compensation enable. */
        uint64_t reserved_4_6          : 3;
        uint64_t tx_pkt_rst_n          : 1;  /**< [  3:  3](R/W) Packet reset for TX. */
        uint64_t tx_dat_rst_n          : 1;  /**< [  2:  2](R/W) Datapath reset for TX. */
        uint64_t rx_pkt_rst_n          : 1;  /**< [  1:  1](R/W) Packet reset for RX. */
        uint64_t rx_dat_rst_n          : 1;  /**< [  0:  0](R/W) Datapath reset for RX. */
#else /* Word 0 - Little Endian */
        uint64_t rx_dat_rst_n          : 1;  /**< [  0:  0](R/W) Datapath reset for RX. */
        uint64_t rx_pkt_rst_n          : 1;  /**< [  1:  1](R/W) Packet reset for RX. */
        uint64_t tx_dat_rst_n          : 1;  /**< [  2:  2](R/W) Datapath reset for TX. */
        uint64_t tx_pkt_rst_n          : 1;  /**< [  3:  3](R/W) Packet reset for TX. */
        uint64_t reserved_4_6          : 3;
        uint64_t comp                  : 1;  /**< [  7:  7](R/W) Compensation enable. */
        uint64_t reserved_8_10         : 3;
        uint64_t dllrst                : 1;  /**< [ 11: 11](R/W) DLL reset. */
        uint64_t reserved_12_14        : 3;
        uint64_t clkrst                : 1;  /**< [ 15: 15](R/W) DLL CLK reset. [CLKRST] must be set if DLL bypass mode
                                                                 XCV_DLL_CTL[CLKRX_BYP,CLKTX_BYP] is used. */
        uint64_t reserved_16_62        : 47;
        uint64_t enable                : 1;  /**< [ 63: 63](R/W) Port enable. */
#endif /* Word 0 - End */
    } s;
    /* struct cavm_xcvx_reset_s cn; */
};

/**
 * Register (RSL) xcv#_ctl
 *
 * XCV Control Register
 * This register contains the status control bits.
 */
union cavm_xcvx_ctl
{
    uint64_t u;
    struct cavm_xcvx_ctl_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_4_63         : 60;
        uint64_t lpbk_ext              : 1;  /**< [  3:  3](R/W) Enable external loopback mode. External loopback loops the RX datapath to the TX
                                                                 datapath. For correct operation, the following CSRs must be configured:
                                                                 * XCV_CTL[SPEED]          = 0x2.
                                                                 * XCV_DLL_CTL[REFCLK_SEL] = 1.
                                                                 * XCV_RESET[CLKRST]       = 1. */
        uint64_t lpbk_int              : 1;  /**< [  2:  2](R/W) Enable internal loopback mode. Internal loopback loops the TX datapath to the RX
                                                                 datapath. For correct operation, the following CSRs must be configured:
                                                                 * XCV_CTL[SPEED]          = 0x2.
                                                                 * XCV_DLL_CTL[REFCLK_SEL] = 0.
                                                                 * XCV_RESET[CLKRST]       = 0. */
        uint64_t speed                 : 2;  /**< [  1:  0](R/W) XCV operational speed:
                                                                   0x0 = 10 Mbps.
                                                                   0x1 = 100 Mbps.
                                                                   0x2 = 1 Gbps.
                                                                   0x3 = Reserved. */
#else /* Word 0 - Little Endian */
        uint64_t speed                 : 2;  /**< [  1:  0](R/W) XCV operational speed:
                                                                   0x0 = 10 Mbps.
                                                                   0x1 = 100 Mbps.
                                                                   0x2 = 1 Gbps.
                                                                   0x3 = Reserved. */
        uint64_t lpbk_int              : 1;  /**< [  2:  2](R/W) Enable internal loopback mode. Internal loopback loops the TX datapath to the RX
                                                                 datapath. For correct operation, the following CSRs must be configured:
                                                                 * XCV_CTL[SPEED]          = 0x2.
                                                                 * XCV_DLL_CTL[REFCLK_SEL] = 0.
                                                                 * XCV_RESET[CLKRST]       = 0. */
        uint64_t lpbk_ext              : 1;  /**< [  3:  3](R/W) Enable external loopback mode. External loopback loops the RX datapath to the TX
                                                                 datapath. For correct operation, the following CSRs must be configured:
                                                                 * XCV_CTL[SPEED]          = 0x2.
                                                                 * XCV_DLL_CTL[REFCLK_SEL] = 1.
                                                                 * XCV_RESET[CLKRST]       = 1. */
        uint64_t reserved_4_63         : 60;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_xcvx_ctl_s cn; */
};


/**
 * Register (RSL) xcv#_batch_crd_ret
 *
 * XCV Batch Credit Return Register
 */
union cavm_xcvx_batch_crd_ret
{
    uint64_t u;
    struct cavm_xcvx_batch_crd_ret_s
    {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
        uint64_t reserved_1_63         : 63;
        uint64_t crd_ret               : 1;  /**< [  0:  0](WO) In case of the reset event, when this register is written XCV sends out the
                                                                 initial credit batch to BGX. Initial credit value of 16. The write will only
                                                                 take effect when XCV_RESET[ENABLE] is set. */
#else /* Word 0 - Little Endian */
        uint64_t crd_ret               : 1;  /**< [  0:  0](WO) In case of the reset event, when this register is written XCV sends out the
                                                                 initial credit batch to BGX. Initial credit value of 16. The write will only
                                                                 take effect when XCV_RESET[ENABLE] is set. */
        uint64_t reserved_1_63         : 63;
#endif /* Word 0 - End */
    } s;
    /* struct cavm_xcvx_batch_crd_ret_s cn; */
};




#endif /* OCTEONTX_XCV_H_ */
