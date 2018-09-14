/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */


#include <common.h>
#include <net.h>
#include <netdev.h>
#include <malloc.h>
#include <dm.h>
#include <misc.h>
#include <pci.h>
#include <memalign.h>
#include <watchdog.h>
#include <asm/io.h>
#include <linux/types.h>
#include <linux/bitops.h>
#include <asm/arch/octeontx2.h>
#include "cavm-csrs-npc.h"
#include "cavm-csrs-nix.h"
#include "npc_profile.h"
#include "nix_af.h"
#include "nix.h"
#include "npc.h"

#define RSVD_MCAM_ENTRIES_PER_PF	2 /** Ucast & Bcast */
#define RSVD_MCAM_ENTRIES_PER_NIXLF	1 /** Ucast for VFs */
#if 0
static u64 npc_af_reg_read(struct npc_af *npc, u64 offset)
{
	return readq(npc->npc_af_base + offset);
}

static void npc_af_reg_write(struct npc_af *npc, u64 offset, u64 val)
{
	writeq(va, npc->npc_af_base + offset);
}
#endif
static inline u64 enable_mask(int count)
{
	return ((count < 64) ? ~(BIT_ULL(count) - 1) : (0x00ULL));
}


#define LDATA_EXTRACT_CONFIG(intf, lid, ltype, ld, cfg) \
	npc_af_reg_write(npc,			\
		CAVM_NPC_AF_INTFX_LIDX_LTX_LDX_CFG(intf, lid, ltype, ld), cfg)

#define LDATA_FLAGS_CONFIG(intf, ld, flags, cfg)	\
	npc_af_reg_write(npc,			\
		CAVM_NPC_AF_INTFX_LDATAX_FLAGSX_CFG(intf, ld, flags), cfg)

static void npc_af_config_layer_info(struct npc_af *npc)
{
	union cavm_npc_af_const af_const;
	union cavm_npc_af_intfx_lidx_ltx_ldx_cfg cfg;
	int lid_count;
	int lid, ltype;
	struct npc_mcam *mcam = &npc->mcam;

	af_const.u = npc_af_reg_read(npc, CAVM_NPC_AF_CONST());
	lid_count = af_const.s.lids;

	/* First clear any existing config i.e
	 * disable LDATA and FLAGS extraction.
	 */
	for (lid = 0; lid < lid_count; lid++) {
		for (ltype = 0; ltype < 16; ltype++) {
			LDATA_EXTRACT_CONFIG(NIX_INTF_RX, lid, ltype, 0, 0ULL);
			LDATA_EXTRACT_CONFIG(NIX_INTF_RX, lid, ltype, 1, 0ULL);
			LDATA_EXTRACT_CONFIG(NIX_INTF_TX, lid, ltype, 0, 0ULL);
			LDATA_EXTRACT_CONFIG(NIX_INTF_TX, lid, ltype, 1, 0ULL);

			LDATA_FLAGS_CONFIG(NIX_INTF_RX, 0, ltype, 0ULL);
			LDATA_FLAGS_CONFIG(NIX_INTF_RX, 1, ltype, 0ULL);
			LDATA_FLAGS_CONFIG(NIX_INTF_TX, 0, ltype, 0ULL);
			LDATA_FLAGS_CONFIG(NIX_INTF_TX, 1, ltype, 0ULL);
		}
	}

	/* If we plan to extract Outer IPv4 tuple for TCP/UDP pkts
	 * then 112bit key is not sufficient
	 */
	if (mcam->keysize != NPC_MCAM_KEY_X2)
		return;

	/* Start placing extracted data/flags from 64bit onwards, for now */
	/* Extract DMAC from the packet */
	cfg.u = 0;
	cfg.s.bytesm1 = 0x05;
	cfg.s.ena = 1;
	cfg.s.key_offset = 0x8;
	LDATA_EXTRACT_CONFIG(NIX_INTF_RX, CAVM_NPC_LID_E_LA, NPC_LT_LA_ETHER, 0, cfg.u);
}

static void npc_af_config_kpuaction(struct npc_af *npc,
				    struct npc_kpu_profile_action *kpuaction,
				    int kpu, int entry, bool pkind)
{
	u64 reg;
	union cavm_npc_af_pkindx_action0 action0;
	union cavm_npc_af_pkindx_action1 action1;

	action0.u = 0;
	action1.u = 0;
	action1.s.errlev = kpuaction->errlev;
	action1.s.errcode = kpuaction->errcode;
	action1.s.dp0_offset = kpuaction->dp0_offset;
	action1.s.dp1_offset = kpuaction->dp1_offset;
	action1.s.dp2_offset = kpuaction->dp2_offset;

	reg = pkind ? CAVM_NPC_AF_PKINDX_ACTION1(entry) :
		      CAVM_NPC_AF_KPUX_ENTRYX_ACTION1(kpu, entry);
	npc_af_reg_write(npc, reg, action1.u);

	action0.s.byp_count = kpuaction->bypass_count;
	action0.s.capture_ena = kpuaction->cap_ena;
	action0.s.parse_done = kpuaction->parse_done;
	action0.s.next_state = kpuaction->next_state;
	action0.s.capture_lid = kpuaction->lid;
	action0.s.capture_ltype = kpuaction->ltype;
	action0.s.capture_flags = kpuaction->flags;
	action0.s.ptr_advance = kpuaction->ptr_advance;
	action0.s.var_len_offset = kpuaction->offset;
	action0.s.var_len_mask = kpuaction->mask;
	action0.s.var_len_right = kpuaction->right;
	action0.s.var_len_shift = kpuaction->shift;

	reg = pkind ? CAVM_NPC_AF_PKINDX_ACTION0(entry) :
		      CAVM_NPC_AF_KPUX_ENTRYX_ACTION0(kpu, entry);
	npc_af_reg_write(npc, reg, action0.u);
}

static void npc_af_config_kpucam(struct npc_af *npc,
				 struct npc_kpu_profile_cam *kpucam,
				 int kpu, int entry)
{
	union cavm_npc_af_kpux_entryx_camx cam0, cam1;

	cam0.u = 0;
	cam1.u = 0;
	cam1.s.state = kpucam->state & kpucam->state_mask;
	cam1.s.dp0_data = kpucam->dp0 & kpucam->dp0_mask;
	cam1.s.dp1_data = kpucam->dp1 & kpucam->dp1_mask;
	cam1.s.dp2_data = kpucam->dp2 & kpucam->dp2_mask;

	cam0.s.state = ~kpucam->state & kpucam->state_mask;
	cam0.s.dp0_data = ~kpucam->dp0 & kpucam->dp0_mask;
	cam0.s.dp1_data = ~kpucam->dp1 & kpucam->dp1_mask;
	cam0.s.dp2_data = ~kpucam->dp2 & kpucam->dp2_mask;

	npc_af_reg_write(npc, CAVM_NPC_AF_KPUX_ENTRYX_CAMX(kpu, entry, 0),
			 cam0.u);
	npc_af_reg_write(npc, CAVM_NPC_AF_KPUX_ENTRYX_CAMX(kpu, entry, 1),
			 cam1.u);
}

static void npc_af_program_kpu_profile(struct npc_af *npc, int kpu,
				       struct npc_kpu_profile *profile)
{
	int entry, num_entries, max_entries;
	union cavm_npc_af_const1 af_const1;
	union cavm_npc_af_kpux_cfg kpu_cfg;

	if (profile->cam_entries != profile->action_entries) {
		printf("%s: KPU%d: CAM and action entries [%d != %d] not equal\n",
		       __func__, kpu, profile->cam_entries,
		       profile->action_entries);
		return;
	}

	af_const1.u = npc_af_reg_read(npc, CAVM_NPC_AF_CONST1());
	max_entries = af_const1.s.kpu_entries;

	/* Program CAM match entries for previous KPU extracted data */
	num_entries = min_t(int, profile->cam_entries, max_entries);
	for (entry = 0; entry < num_entries; entry++)
		npc_af_config_kpucam(npc, &profile->cam[entry], kpu, entry);

	/* Program this KPU's actions */
	num_entries = min_t(int, profile->action_entries, max_entries);
	npc_af_reg_write(npc, CAVM_NPC_AF_KPUX_ENTRY_DISX(kpu, 0),
			 enable_mask(num_entries));
	if (num_entries > 64)
		npc_af_reg_write(npc, CAVM_NPC_AF_KPUX_ENTRY_DISX(kpu, 1),
				 enable_mask(num_entries - 64));

	/* Enable this KPU */
	kpu_cfg.u = npc_af_reg_read(CAVM_NPC_AF_KPUX_CFG(kpu));
	kpu_cfg.s.ena = 1;
	npc_af_reg_write(npc, CAVM_NPC_AF_KPUX_CFG(kpu), kpu_cfg.u);
}

static void npc_parser_profile_init(struct npc_af *npc)
{
	int num_pkinds, num_kpus, idx;
	struct npc_pkind *pkind = &npc->pkind;
	union cavm_npc_af_const af_const;

	af_const.u = npc_af_reg_read(npc, CAVM_NPC_AF_CONST());
	npc->npc_kpus = af_const.s.kpus;

	/* Disable all KPUs and their entries */
	for (idx = 0; idx < npc->npc_kpus; idx++) {
		npc_af_reg_write(npc, CAVM_NPC_AF_KPUX_ENTRY_DISX(idx, 0),
				 ~0ULL);
		npc_af_reg_write(npc, CAVM_NPC_AF_KPUX_ENTRY_DISX(idx, 1),
				 ~0ULL);
		npc_af_reg_write(npc, CAVM_NPC_AF_KPUX_CFG(idx), 0);
	}

	/* First program IKPU profile (i.e. PKIND configs).
	 * Check HW max count to avoid configuring junk or writing to
	 * unsupported CSR addresses.
	 */

	num_pkinds = ARRAY_SIZE(ikpu_action_entries);
	num_pkinds = min_t(int, pkind->rsrc.max, num_pkinds);

	for (idx = 0; idx < num_pkinds; idx++)
		npc_af_config_kpuaction(npc, &ikpu_action_entries[idx],
					0, idx, true);
	/* Program KPU CAM and Action profiles */
	num_kpus = ARRAY_SIZE(npc_kpu_profiles);
	num_kpus = min_t(int, npc->npc_kpus, num_kpus);

	for (idx = 0; idx < num_kpus; idx++)
		npc_af_program_kpu_profile(npc, idx, &npc_kpu_profiles[idx]);

	npc_af_config_layer_info(npc);
}

static int npc_af_mcam_rsrcs_init(struct npc_af *npc)
{
	int err, rsvd;
	struct npc_mcam *mcam = &npc->mcam;
	union cavm_npc_af_const af_const;
	union cavm_npc_af_intfx_kex_cfg kex_cfg;
	struct nix_af_handle *nix = npc->nix_af;

	/* Get HW limits */
	af_const.u = npc_af_reg_read(npc, CAVM_NPC_AF_CONST());
	mcam->banks = af_const.s.mcam_banks;
	mcam->banksize = af_const.s.mcam_bank_depth;
	mcam->counters = af_const.s.match_stats;

	/* Actual number of MCAM entries vary by entry size */
	kex_cfg.u = npc_af_reg_read(npc, CAVM_NPC_AF_INTFX_KEX_CFG(0));
	mcam->total_entries = (mcam->banks / kex_cfg.s.keyw) * mcam->banksize;
	mcam->keysize = kex_cfg.s.keyw;

	/* Number of banks combined per MCAM entry */
	switch (kex_cfg.s.keyw) {
	case CAVM_NPC_MCAMKEYW_E_X4:
		mcam->banks_per_entry = 4;
		break;
	case CAVM_NPC_MCAMKEYW_E_X2:
		mcam->banks_per_entry = 2;
		break;
	default:
		mcam->banks_per_entry = 1;
		break;
	}

	/* Reserve one MCAM entry for each of the NIX LFs to guarantee space
	 * to install default matching DMAC rule.  Also reserve 2 MCAM entries
	 * for each PF for default channel based matchingor 'ucast & bcast'
	 * matching to support UCAST and PROMISC modes of operation for PFs.
	 * PF0 is excluded.
	 */
	rsvd = (npc->block.rsrc.max * RSVD_MCAM_ENTRIES_PER_NIXLF) +
		((npc->hw->total_pfs - 1) * RSVD_MCAM_ENTRIES_PER_PF);
	if (mcam->total_entries <= rsvd) {
		printf("%s: Insufficient NPC MCAM size %d for pkt I/O, exiting\n",
		       __func__);
		return -ENOMEM;
	}
	mcam->entries = mcam->total_entries - rsvd;
	mcam->nixlf_offset = mcam->entries;
	mcam->pf_offset = mcam->nixlf_offset + nix->rsrc.max;

	/* Allocate bitmap for this resource and memory for MCAM entry to
	 * RVU PFFUNC allocation mapping info.
	 */
	mcam->rsrc.max = mcam->entries;
	err = rvu_alloc_bitmap(&mcam->rsrc);
	if (err)
		return err;
	mcam->pfvf_map = calloc(mcam->rsrc.max, sizeof(u16));
	if (!mcam->pfvf_map)
		return -ENOMEM;

	return 0;
}

int npc_af_init(struct npc_af *npc)
{
	int err;
	u64 keyz = NPC_MCAM_KEY_X2;
	struct npc_pkind *pkind = &npc->pkind;
	union cavm_npc_af_const1 af_const1;
	union cavm_npc_af_pck_def_ol2 pck_ol2;
	union cavm_nixx_af_rx_def_ol2 rx_def_ol2;
	union cavm_nixx_af_rx_def_oudp rx_oudp;
	union cavm_nixx_af_rx_def_otcp rx_otcp;
	union cavm_nixx_af_rx_def_oip4 rx_oip4;
	union cavm_npc_af_pck_def_oip4 pck_oip4;
	union cavm_npc_af_pck_cfg pck_cfg;
	union cavm_npc_af_intfx_kex_cfg key_cfg;

	af_const1.u = npc_af_reg_read(npc, CAVM_NPC_AF_CONST1());
	pkind->rsrc.max = af_const1.s.pkinds;

	err = rvu_alloc_bitmap(&pkind->rsrc);
	if (err)
		return err;

	pkind->pfchan_map = calloc(pkind->rsrc.max, sizeof(u32));
	if (!pkind->pfchan_map)
		return -ENOMEM;

	/* Configure KPU profile */
	npc_parser_profile_init(npc);

	/* Config outer L2, IP, TCP and UDP's NPC layer info */
	pck_ol2.u = 0;
	pck_ol2.s.lid = CAVM_NPC_LID_E_LA;
	pck_ol2.s.ltype_match = NPC_LT_LA_ETHER;
	pck_ol2.s.ltype_mask = 0x0f;
	npc_af_reg_write(npc, CAVM_NPC_AF_PCK_DEF_OL2(), pck_ol2.u);
	rx_def_ol2.u = 0;
	rx_def_ol2.s.lid = CAVM_NPC_LID_E_LA;
	rx_def_ol2.s.ltype_match = NPC_LT_LA_ETHER;
	rx_def_ol2.s.ltype_mask = 0x0f;
	nix_af_reg_write(npc->nix_af, CAVM_NIXX_AF_RX_DEF_OL2(), rx_def_ol2.u);
	rx_oudp.u = 0;
	rx_oudp.s.lid = CAVM_NPC_LID_E_LD;
	rx_oudp.s.ltype_match = NPC_LT_LD_UDP;
	rx_oudp.s.ltype_mask = 0x0f;
	nix_af_reg_write(npc->nix_af, CAVM_NIXX_AF_RX_DEF_OUDP(), rx_oudp.u);
	rx_otcp.s.lid = CAVM_NPC_LID_E_LD;
	rx_otcp.s.ltype_match = NPC_LT_LD_TCP;
	rx_otcp.s.ltype_mask = 0x0f;
	nix_af_reg_write(npc->nix_af, CAVM_NIXX_AF_RX_DEF_OTCP(), rx_otcp.u);
	rx_oip4.u = 0;
	rx_oip4.s.lid = CAVM_NPC_LID_E_LC;
	rx_oip4.s.ltype_match = NPC_LT_LC_IP;
	rx_oip4.s.ltype_mask = 0x0f;
	nix_af_reg_write(npc->nix_af, CAVM_NIXX_AF_RX_DEF_OIP4(), rx_oip4.u);
	pck_oip4.u = 0;
	pck_oip4.s.lid = CAVM_NPC_LID_E_LC;
	pck_oip4.s.ltype_match = NPC_LT_LC_IP;
	pck_oip4.s.ltype_mask = 0x0f;
	npc_af_reg_write(npc, CAVM_NPC_AF_PCK_DEF_OIP4(), pck_oip4.u);

	/* Enable below for Rx packets
	 * - IPv4 header checksum validation
	 * - Detect outer L2 broadcast addresses/
	 */
	pck_cfg.u = npc_af_reg_read(npc, CAVM_NPC_AF_PCK_CFG());
	pck_cfg.s.oip4_cksum = 1;
	pck_cfg.s.l2b = 1;
	npc_af_reg_write(npc, CAVM_NPC_AF_PCK_CFG(), pck_cfg.u)

	/* Set RX and TX side MCAM search key size.
	 * Also enable parse key extract nibles such that execpt layer E to H,
	 * rest of the key is included for MCAM search.
	 * Layer E-H is excluded since all KPUs are not yet in use.
	 */
	key_cfg.u = 0;
	key_cfg.s.keyw = keyz & 0x3;
	key_cfg.s.parse_nibble_ena = (1 << 20) - 1;
	npc_af_reg_write(npc,
		      CAVM_NPC_AF_INTFX_KEY_CFG(CAVM_NPC_INTF_E_NIXX_RX(0)), key_cfg.u);
	key_cfg.u = 0;
	key_cfg.s.keyw = keyz & 0x3;
	key_cfg.s.parse_nibble_ena = (1 << 20) - 1;
	npc_af_reg_write(npc,
		      CAVM_NPC_AF_INTFX_KEY_CFG(CAVM_NPC_INTF_E_NIXX_TX(0)),
		      key_cfg.u);

	/* Set TX miss action to UCAST_DEFAULT, i.e. transmit the packet on
	 * NIX LF SQ's default channel
	 */
	npc_af_reg_write(npc,
		      CAVM_NPC_AF_INTFX_MISS_ACT(CAVM_NPC_INTF_E_NIXX_TX(0)),
		      CAVM_NIX_TX_ACTIONOP_E_UCAST_DEFAULT);

	/* If MCAM lookup doesn't result in a match, drop the received packet */
	npc_af_reg_write(npc,
		      CAVM_NPC_AF_INTFX_MISS_ACT(CAVM_NPC_INTF_E_NIXX_RX(0)),
		      CAVM_NIX_RX_ACTIONOP_E_DROP);

	err = npc_mcam_rsrcs_init(npc);

	return err;
}
