// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2024 Marvell
 *
 * https://spdx.org/licenses
 */
#include <common.h>
#include <net.h>
#include <malloc.h>
#include <dm.h>
#include <misc.h>
#include <asm/io.h>
#include <errno.h>
#include <linux/list.h>
#include <asm/arch/board.h>

#include "rpm.h"

char lmac_type_to_str[][8] = {
	"SGMII",
	"XAUI",
	"RXAUI",
	"10G_R",
	"40G_R",
	"RGMII",
	"QSGMII",
	"25G_R",
	"50G_R",
	"100G_R",
	"USXGMII",
	"USGMII",
};

char lmac_speed_to_str[][8] = {
	"0",
	"10M",
	"100M",
	"1G",
	"2.5G",
	"5G",
	"10G",
	"20G",
	"25G",
	"40G",
	"50G",
	"80G",
	"100G",
};

extern u64 get_fwdata_base(void);

void print_fwdata_lmac_type(int rpm_id, int lmac_id, u8 rpm_v2)
{
	int lmac_type, portm_idx;
	u64 fwdata_base = get_fwdata_base();
	struct sh_fwdata *p_sh_fwdata = (struct sh_fwdata *)fwdata_base;
	struct eth_lmac_fwdata_s *data;

	if (!fwdata_base)
		return;

	BUILD_BUG_ON(offsetof(struct sh_fwdata, eth_fw_data) > FWDATA_CGX_LMAC_OFFSET);

	data = rpm_v2 ? &p_sh_fwdata->eth_fw_data_usx[rpm_id][lmac_id] :
			&p_sh_fwdata->eth_fw_data[rpm_id][lmac_id];
	lmac_type = data->lmac_type;
	portm_idx = data->portm_idx;
	printf("PORTM%d: RPM%d LMAC%d [%s]\n",
	       portm_idx, rpm_id, lmac_id,
	       lmac_type_to_str[lmac_type]);
}

static inline void mcs_write(u64 val, u64 offset)
{
	writeq(val, MCS_BASE + offset);
}

static inline u64 mcs_read(u64 offset)
{
	return readq(MCS_BASE + offset);
}

void mcs_init(void)
{
	u64 reg;

	/*
	 * Calibrate X2P interface
	 */
	reg = mcs_read(MCS_MIL_RX_GBL_STS);
	reg &= MCS_MIL_RX_GBL_CLB_DONE;
	if (!reg) {
		reg = mcs_read(MCS_MIL_GLOBAL);
		reg |= MCS_MIL_GLOBAL_CLB_X2P;
		mcs_write(reg, MCS_MIL_GLOBAL);

		reg = mcs_read(MCS_MIL_RX_GBL_STS);
		while (!(reg & MCS_MIL_RX_GBL_CLB_DONE))
			reg = mcs_read(MCS_MIL_RX_GBL_STS);

		reg = mcs_read(MCS_MIL_GLOBAL);
		reg &= ~MCS_MIL_GLOBAL_CLB_X2P;
		mcs_write(reg, MCS_MIL_GLOBAL);
		reg = mcs_read(MCS_MIL_GLOBAL);
	}
}

int get_mgmt_port_pf_idx(void)
{
	bool mgmt_port_set = false;
	int i, rpmid, lmacid, maxlmac, pf_idx = -1;
	unsigned int pf_devid;
	struct udevice *rdev;
	int err = 0;
	u64 fwdata_base = get_fwdata_base();
	struct sh_fwdata *p_sh_fwdata = (struct sh_fwdata *)fwdata_base;
	struct eth_lmac_fwdata_s *p_eth_fwdata = NULL;
	struct rpm *rpm;

	maxlmac = 8;
	pf_devid = PCI_DEVICE_ID_CAVIUM_RPM2;

	for (rpmid = 0; rpmid < MAX_RPM; rpmid++) {
		for (lmacid = 0; lmacid < maxlmac; lmacid++) {
				p_eth_fwdata = &p_sh_fwdata->eth_fw_data_usx[rpmid][lmacid];
			if (p_eth_fwdata->mgmt_port) {
				mgmt_port_set = true;
				break;
			}
		}
		if (mgmt_port_set)
			break;
	}
	if (mgmt_port_set) {
		for (i = 0; i < MAX_RPM; i++) {
			err = dm_pci_find_device(PCI_VENDOR_ID_CAVIUM,
						 pf_devid, i, &rdev);
			if (err) {
				debug("%s RPM%d device not found\n", __func__, i);
				continue;
			} else {
				if (rdev) {
					rpm = dev_get_priv(rdev);
					if (rpm->rpm_id == rpmid) {
						pf_idx = rpm->lmac[lmacid]->instance;
						break;
					}
				}
			}
		}
	}
	debug("%s RPM%d LMAC%d PF %d\n", __func__, rpmid, lmacid, pf_idx);
	return pf_idx;
}

/**
 * Given an LMAC/PF instance number, return the lmac
 * Per design, each PF has only one LMAC mapped.
 *
 * @param instance	instance to find
 *
 * @return	pointer to lmac data structure or NULL if not found
 */
struct lmac *nix_get_rpm_lmac(int lmac_instance)
{
	struct rpm *rpm;
	struct udevice *dev;
	int i, idx, err;
	unsigned int devid = PCI_DEVICE_ID_CAVIUM_RPM;

	devid = PCI_DEVICE_ID_CAVIUM_RPM2;

	for (i = 0; i < MAX_RPM; i++) {
		err = dm_pci_find_device(PCI_VENDOR_ID_CAVIUM,
					 devid, i,
					 &dev);
		if (err)
			continue;

		rpm = dev_get_priv(dev);
		debug("%s udev %p rpm %p instance %d\n", __func__, dev, rpm,
		      lmac_instance);
		for (idx = 0; idx < rpm->max_lmac; idx++) {
			if (rpm->lmac[idx] &&
			    rpm->lmac[idx]->instance == lmac_instance)
				return rpm->lmac[idx];
		}
	}
	return NULL;
}

void rpm_lmac_mac_filter_clear(struct lmac *lmac)
{
	union rpmx_cmrx_rx_dmac_ctl0 dmac_ctl0;
	union rpmx_cmr_rx_dmacx_cam0 dmac_cam0;
	void *reg_addr;

	dmac_cam0.u = 0x0;
	reg_addr = lmac->rpm->reg_base +
			RPMX_CMR_RX_DMACX_CAM0(lmac->lmac_id * 8);
	writeq(dmac_cam0.u, reg_addr);
	debug("%s: reg %p dmac_cam0 %llx\n", __func__, reg_addr, dmac_cam0.u);

	dmac_ctl0.u = 0x0;
	dmac_ctl0.s.bcst_accept = 1;
	dmac_ctl0.s.mcst_mode = 1;
	dmac_ctl0.s.cam_accept = 0;
	reg_addr = lmac->rpm->reg_base +
			RPMX_CMRX_RX_DMAC_CTL0(lmac->lmac_id);
	writeq(dmac_ctl0.u, reg_addr);
	debug("%s: reg %p dmac_ctl0 %llx\n", __func__, reg_addr, dmac_ctl0.u);
}

void rpm_lmac_mac_filter_setup(struct lmac *lmac)
{
	union rpmx_cmrx_rx_dmac_ctl0 dmac_ctl0;
	union rpmx_cmr_rx_dmacx_cam0 dmac_cam0;
	u64 mac, tmp;
	void *reg_addr;

	memcpy((void *)&tmp, lmac->mac_addr, 6);
	mac = swab64(tmp) >> 16;
	debug("%s: mac %llx\n", __func__, mac);
	dmac_cam0.u = 0x0;
	if (lmac->rpm->is_v2) {
		dmac_cam0.s.id = lmac->lmac_id;
		dmac_cam0.s.adr = mac;
		dmac_cam0.s.en = 1;
	}

	reg_addr = lmac->rpm->reg_base +
			RPMX_CMR_RX_DMACX_CAM0(lmac->lmac_id * 8);
	writeq(dmac_cam0.u, reg_addr);
	debug("%s: reg %p dmac_cam0 %llx\n", __func__, reg_addr, dmac_cam0.u);
	dmac_ctl0.u = 0x0;
	dmac_ctl0.s.bcst_accept = 1;
	dmac_ctl0.s.mcst_mode = 0;
	dmac_ctl0.s.cam_accept = 1;
	reg_addr = lmac->rpm->reg_base +
			RPMX_CMRX_RX_DMAC_CTL0(lmac->lmac_id);
	writeq(dmac_ctl0.u, reg_addr);
	debug("%s: reg %p dmac_ctl0 %llx\n", __func__, reg_addr, dmac_ctl0.u);
}

int rpm_lmac_set_chan(struct lmac *lmac)
{
	union rpmx_cmrx_link_cfg link_cfg;
	u64 reg, offset;

	link_cfg.u = 0;
	link_cfg.s.log2_range = 0x4;
	link_cfg.s.base_chan = lmac->chan_num;
	rpm_write(lmac->rpm, RPMX_CMRX_LINK_CFG(lmac->lmac_id),
		  link_cfg.u);

	/*
	 * Set MCS channel numbers
	 */
	if (lmac->rpm->is_v2) {
		offset = MCS_LINK_LMACX_CFG(lmac->rpm->rpm_id * 0x8 +
					    lmac->lmac_id);
		reg = mcs_read(offset);
		reg &= ~GENMASK_ULL(11, 0);
		reg |= (u64)lmac->chan_num;
		mcs_write(reg, offset);
	}

	return 0;
}

int rpm_lmac_set_pkind(struct lmac *lmac, u8 lmac_id, int pkind)
{
	rpm_write(lmac->rpm, RPMX_CMRX_RX_ID_MAP(lmac_id),
		  (pkind & 0x3f));
	return 0;
}

int rpm_lmac_link_status(struct lmac *lmac, int lmac_id, u64 *status)
{
	int ret = 0;

	ret = eth_intf_get_link_sts(lmac->rpm->rpm_id, lmac_id, status);
	if (ret) {
		debug("%s request failed for rpm%d lmac%d\n",
		      __func__, lmac->rpm->rpm_id, lmac->lmac_id);
		ret = -1;
	}
	return ret;
}

int rpm_lmac_rx_tx_enable(struct lmac *lmac, int lmac_id, bool enable)
{
	struct rpm *rpm = lmac->rpm;
	union rpmx_mti_mac100x_command_config command_config;

	if (!rpm || lmac_id >= rpm->lmac_count)
		return -ENODEV;

	command_config.u = rpm_read(rpm,
				    RPMX_MTI_MAC100X_COMMAND_CONFIG(lmac_id));
	command_config.s.rx_ena =
	command_config.s.tx_ena = enable ? 1 : 0;
	rpm_write(rpm, RPMX_MTI_MAC100X_COMMAND_CONFIG(lmac_id),
		  command_config.u);
	return 0;
}

int rpm_lmac_link_enable(struct lmac *lmac, int lmac_id, bool enable,
			 u64 *status)
{
	int ret = 0;

	ret = eth_intf_link_up_dwn(lmac->rpm->rpm_id, lmac_id, enable,
				   status);
	if (ret) {
		debug("%s request failed for rpm%d lmac%d\n",
		      __func__, lmac->rpm->rpm_id, lmac->lmac_id);
		ret = -1;
	}
	return ret;
}

static int rpm_lmac_init(struct rpm *rpm)
{
	struct lmac *lmac;
	static int instance = 1;	//PF0 is AF
	union rpmx_cmrx_config cmrx_cfg;
	union rpmx_cmr_rx_lmacs rx_lmacs;
	union rpmx_const rpm_const;
	int i, lmac_exist, mac_index;

	rpm_const.u = rpm_read(rpm, RPMX_CONST());
	rpm->max_lmac = rpm_const.s.lmacs;
	rpm->is_v2 = (rpm_const.s.ver == 2);

	rx_lmacs.u = rpm_read(rpm, RPMX_CMR_RX_LMACS());
	lmac_exist = rx_lmacs.s.lmac_exist;

	for (i = 0; i < rpm->max_lmac; i++)
		if (lmac_exist & BIT(i))
			rpm->lmac_count++;
	debug("%s: Found %d lmacs for rpm %d@%p\n", __func__, rpm->lmac_count,
	      rpm->rpm_id, rpm->reg_base);

	for (i = 0; i < fls(lmac_exist); i++) {
		if (!(rx_lmacs.u & BIT(i)))
			continue;
		lmac = calloc(1, sizeof(*lmac));
		if (!lmac)
			return -ENOMEM;
		lmac->instance = instance++;
		snprintf(lmac->name, sizeof(lmac->name), "rpm_fwi_%d_%d",
			 rpm->rpm_id, i);

		cmrx_cfg.u = rpm_read(rpm, RPMX_CMRX_CONFIG(i));
		lmac->p2x_sel = cmrx_cfg.s.p2x_select;
		cmrx_cfg.s.enable = 1;
		rpm_write(rpm, RPMX_CMRX_CONFIG(i), cmrx_cfg.u);

		lmac->lmac_id = i;
		lmac->rpm = rpm;
		rpm->lmac[i] = lmac;
		debug("%s: map id %d to lmac %p (%s), instance %d\n",
		      __func__, i, lmac, lmac->name, lmac->instance);
		lmac->init_pend = 1;
		mac_index = rpm->rpm_id * (rpm_const.s.ver * 4) + i;
		cn20k_board_get_mac_addr(mac_index, lmac->mac_addr);
		debug("%s: MAC %pM\n", __func__, lmac->mac_addr);
		rpm_lmac_mac_filter_setup(lmac);
		print_fwdata_lmac_type(rpm->rpm_id, i, rpm->is_v2);
		rpm_write(rpm, RPMX_CMRX_SCRATCHX(i, 0), 0x0);
	}

	if (rpm->is_v2)
		mcs_init();

	return 0;
}

int rpm_probe(struct udevice *dev)
{
	struct rpm *rpm = dev_get_priv(dev);
	int err;
	static uint8_t rpmid;
	rpm->reg_base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0, 0, 0,
				       PCI_REGION_TYPE, PCI_REGION_MEM);
	rpm->dev = dev;
	rpm->rpm_id = rpmid++;//((u64)(rpm->reg_base) >> 24) & 0xf;

	err = rpm_lmac_init(rpm);

	return err;
}

int rpm_remove(struct udevice *dev)
{
	struct rpm *rpm = dev_get_priv(dev);
	int i;

	for (i = 0; i < rpm->max_lmac; i++)
		if (rpm->lmac[i])
			rpm_lmac_mac_filter_clear(rpm->lmac[i]);

	return 0;
}

U_BOOT_DRIVER(rpm) = {
	.name	= "rpm",
	.id	= UCLASS_MISC,
	.probe	= rpm_probe,
	.remove	= rpm_remove,
	.priv_auto = sizeof(struct rpm),
};

static struct pci_device_id rpm_supported[] = {
	{PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_CAVIUM_RPM) },
	{PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_CAVIUM_RPM2) },
	{}
};

U_BOOT_PCI_DEVICE(rpm, rpm_supported);
