// SPDX-License-Identifier: BSD-2-Clause-Patent
/*
 * https://spdx.org/licenses
 *
 * Copyright (C) 2024 Marvell
 *
 */

#include <common.h>
#include <console.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>
#include <env.h>
#include <init.h>
#include <log.h>
#include <malloc.h>
#include <net.h>
#include <errno.h>
#include <linux/io.h>
#include <linux/compiler.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <asm/arch/smc.h>
#include <asm/arch/soc.h>
#include <asm/arch/board.h>
#include <asm/arch/switch.h>
#include <dm/util.h>
#include <spi.h>
#include <wdt.h>
#include <linux/iopoll.h>

#ifdef CONFIG_FWLOG_TTYMEM
#define FWLOG_CONSOLE_NAME	"ttymem"
#endif

extern ssize_t smc_flsf_fw_booted(void);

DECLARE_GLOBAL_DATA_PTR;

/* U-boot boot status */
#define BOOT_SUCCESS    1
enum boot_error {
	BOOT_NEXT_STAGE_SUCCESS = 0x0,
	BOOT_NEXT_STAGE_ERROR,
	/* U-boot boot successfully and Linux boot is not started yet */
	BOOT_NEXT_STAGE_PENDING,
};

#define BOOTCMD_NAME   "pci-bootcmd"
#define CONSOLE_NAME	"pci-console@0"

#define NUM_SPI_CTRL	2
#define NUM_SPI_CS	2

#ifdef CONFIG_FWLOG_TTYMEM
static int init_ttymem_console(void)
{
	struct udevice *ttymem_dev = NULL;
	int ret = 0;
	char iomux_name[128];
	char *stdoutname = env_get("stdout");
	char *stderrname = env_get("stderr");
	bool stdout_set, stderr_set;

	if (!stdoutname) {
		env_set("stdout", "serial");
		stdoutname = env_get("stdout");
	}
	if (!stderrname) {
		env_set("stderr", "serial");
		stderrname = env_get("stderr");
	}

	if (!stdoutname || !stderrname) {
		printf("%s: Error setting environment variabled for serial\n",
			__func__);
		return -1;
	}

	stdout_set = !!strstr(stdoutname, FWLOG_CONSOLE_NAME);
	stderr_set = !!strstr(stderrname, FWLOG_CONSOLE_NAME);

	printf("stdout: %s \nstderr: %s\n",stdoutname, stderrname);

	ret = uclass_get_device_by_driver(UCLASS_SERIAL,
				DM_DRIVER_GET(serial_ttymem),
				&ttymem_dev);
	if (!ttymem_dev) {
		printf("ttymem dev not found %s\n", ttymem_dev->name);
		return 0;
	}

	if (stdout_set)
		strncpy(iomux_name, stdoutname, sizeof(iomux_name));
	else
		snprintf(iomux_name, sizeof(iomux_name), "%s,%s", stdoutname,
			 ttymem_dev->name);

	ret = iomux_doenv(stdout, iomux_name);
	if (!stdout_set)
		env_set("stdout", iomux_name);

	if (stderr_set)
		strncpy(iomux_name, stderrname, sizeof(iomux_name));
	else
		snprintf(iomux_name, sizeof(iomux_name), "%s,%s", stderrname,
			 ttymem_dev->name);

	ret = iomux_doenv(stderr, iomux_name);
	if (ret) {
		pr_err("%s: Error setting I/O stderr MUX to %s\n",
		       __func__, iomux_name);
		return ret;
	}

	if (!stderr_set)
		env_set("stderr", iomux_name);

	debug("%s: stdout: %s, stderr: %s, ret: %d\n",
	      __func__, env_get("stdout"),
	      env_get("stderr"), ret);
	return ret;
}
#endif

void cleanup_env_ethaddr(void)
{
	char ename[32];

	for (int i = 0; i < 20; i++) {
		sprintf(ename, i ? "eth%daddr" : "ethaddr", i);
		if (env_get(ename))
			env_set(ename, NULL);
	}
}

void cn20k_board_get_mac_addr(u8 index, u8 *mac_addr)
{
	u64 tmp_mac, mac;
	static int mac_num;

	memset(mac_addr, 0, ARP_HLEN);
	mac_num = fdt_get_board_mac_cnt();

	if (mac_num && (index < mac_num)) {
		mac = fdt_get_board_mac_addr(index);
		if (!is_zero_ethaddr((u8 *)&mac)) {
			tmp_mac = swab64(mac) >> 16;
			memcpy(mac_addr, (u8 *)&tmp_mac, ARP_HLEN);
		}
	}
	debug("%s mac %pM\n", __func__, mac_addr);
}

void board_get_spi_bus_cs(struct udevice *dev, int *bus, int *cs)
{
	struct udevice *busp, *csp;
	struct udevice *parent = dev_get_parent(dev);

	for (int i = 0; i < NUM_SPI_CTRL; i++) {
		for (int j = 0; j < NUM_SPI_CS; j++) {
			if (!spi_find_bus_and_cs(i, j, &busp, &csp)) {
				if (parent == busp && dev == csp) {
					*bus = i;
					*cs = j;
					break;
				}
			}
		}
	}
}

void board_get_env_spi_bus_cs(int *bus, int *cs)
{
	const void *blob = gd->fdt_blob;
	int env_bus, env_cs;
	int node, preg;

	env_bus = -1;
	env_cs = -1;
	node = fdt_node_offset_by_compatible(blob, -1, "spi-flash");
	while (node > 0) {
		if (fdtdec_get_bool(blob, node, "u-boot,env")) {
			env_cs = fdtdec_get_int(blob, node, "reg", -1);
			preg = fdtdec_get_int(blob,
					      fdt_parent_offset(blob, node),
					      "reg", -1);
			/* SPI node will have PCI addr, so map it */
			if (preg == 0xCF10)
				env_bus = 0;
			if (preg == 0xCF11)
				env_bus = 1;
			debug("\n Env SPI [bus:cs] [%d:%d]\n",
			      env_bus, env_cs);
			break;
		}
		node = fdt_node_offset_by_compatible(blob, node, "spi-flash");
	}
	if (env_bus == -1)
		debug("\'u-boot,env\' property not found in fdt\n");

	*bus = env_bus;
	*cs = env_cs;
}

void board_get_env_offset(int *offset, const char *property)
{
	const void *blob = gd->fdt_blob;
	int env_offset;
	int node;

	env_offset = -1;
	node = fdt_node_offset_by_compatible(blob, -1, "spi-flash");
	while (node > 0) {
		if (fdtdec_get_bool(blob, node, "u-boot,env")) {
			env_offset = fdtdec_get_int(blob, node, property, -1);
			debug("\n %s : 0x%x\n", property, env_offset);
			break;
		}
		node = fdt_node_offset_by_compatible(blob, node, "spi-flash");
	}
	if (env_offset == -1)
		debug("\n %s property not found in fdt\n", property);

	*offset = env_offset;
}

#if CONFIG_IS_ENABLED(CN20K_FIXED_MGMT_PORT)
extern int get_mgmt_port_pf_idx(void);
void probe_network_mgmt_port(void)
{
	struct udevice *dev;
	int err, dev_idx = 0;
	struct uclass *uc;
	int ret;

	dev_idx = get_mgmt_port_pf_idx();
	if (dev_idx < 0)
		return;

	ret = uclass_get(UCLASS_ETH, &uc);
	if (ret)
		return;
	if (list_empty(&uc->dev_head))
		return;

	/* Move mgmt port device sequence as first */
	uclass_foreach_dev(dev, uc) {
		if (!dev_seq(dev))
			dev->seq_ = dev_idx - 1;
		else if (dev_seq(dev) == (dev_idx - 1))
			dev->seq_ = 0;
	}
}
#else
void probe_network_mgmt_port(void)
{
}
#endif

void probe_network_devices(bool probe)
{
	struct udevice *dev;
	int err, rpm_cnt, i;
	unsigned int devid = PCI_DEVICE_ID_CAVIUM_RPM;

	switch (read_partnum()) {
	case CN20KA:
		devid = PCI_DEVICE_ID_CAVIUM_RPM2;
		rpm_cnt = 3;
		break;
	default:
		rpm_cnt = 0;
		break;
	}
	/* MAC(RPM) and RVU AF devices */
	for (i = 0; i < rpm_cnt; i++) {
		err = dm_pci_find_device(PCI_VENDOR_ID_CAVIUM,
					 devid, i, &dev);
		if (err)
			debug("%s RPM%d device not found\n", __func__, i);
		if (!probe)
			device_remove(dev, DM_REMOVE_NORMAL);
	}
	err = dm_pci_find_device(PCI_VENDOR_ID_CAVIUM,
				 PCI_DEVICE_ID_CAVIUM_RVU_AF, 0, &dev);
	if (err)
		debug("NIC AF device not found\n");
	if (!probe)
		device_remove(dev, DM_REMOVE_NORMAL);

	if (IS_ENABLED(CONFIG_CN20K_FIXED_MGMT_PORT))
		probe_network_mgmt_port();
}

int board_early_init_r(void)
{
	pci_init();
	return 0;
}

int board_init(void)
{
	return 0;
}

int timer_init(void)
{
	return 0;
}

int dram_init(void)
{
	u64 rvu_addr, rvu_size;
	int ret;

	gd->ram_size = smc_dram_size(0);
	/* Initial Secure region, ECP region */
	gd->ram_size += 0x2000000;
	gd->ram_size -= CONFIG_SYS_SDRAM_BASE;

	rvu_addr = 0;
	rvu_size = 0;
	ret = smc_rvu_rsvd_reg_info(&rvu_addr, &rvu_size);

	if (IS_ENABLED(CONFIG_CN20K_MAP_RESERVE))
		board_fdt_get_rsvd_size(&rvu_addr, &rvu_size);

	mem_map_fill(rvu_addr, rvu_size);
	return 0;
}

#if (CONFIG_IS_ENABLED(OCTEONTX_SERIAL_BOOTCMD) ||	\
	CONFIG_IS_ENABLED(OCTEONTX_SERIAL_PCIE_CONSOLE)) &&	\
	!CONFIG_IS_ENABLED(CONSOLE_MUX)
# error CONFIG_CONSOLE_MUX must be enabled!
#endif

#if CONFIG_IS_ENABLED(OCTEONTX_SERIAL_BOOTCMD)
static int init_bootcmd_console(void)
{
	int ret = 0;
	char *stdinname = env_get("stdin");
	struct udevice *bootcmd_dev = NULL;
	bool stdin_set;
	char iomux_name[128];

	debug("%s: stdin before: %s\n", __func__,
	      stdinname ? stdinname : "NONE");
	if (!stdinname) {
		env_set("stdin", "serial");
		stdinname = env_get("stdin");
	}
	stdin_set = !!strstr(stdinname, BOOTCMD_NAME);
	ret = uclass_get_device_by_driver(UCLASS_SERIAL,
					  DM_DRIVER_GET(octeontx_bootcmd),
					  &bootcmd_dev);
	if (ret) {
		pr_err("%s: Error getting %s serial class\n", __func__,
		       BOOTCMD_NAME);
	} else if (bootcmd_dev) {
		if (stdin_set)
			strlcpy(iomux_name, stdinname, sizeof(iomux_name));
		else
			snprintf(iomux_name, sizeof(iomux_name), "%s,%s",
				 stdinname, bootcmd_dev->name);
		ret = iomux_doenv(stdin, iomux_name);
		if (ret)
			pr_err("%s: Error %d enabling the PCI bootcmd input console \"%s\"\n",
			       __func__, ret, iomux_name);
		if (!stdin_set)
			env_set("stdin", iomux_name);
	}
	debug("%s: Set iomux and stdin to %s (ret: %d)\n",
	      __func__, iomux_name, ret);
	return ret;
}
#endif

/**
 * Board late initialization routine.
 */
int board_late_init(void)
{
	char boardname[32];
	char boardserial[150], boardrev[150];
	bool save_env = false;
	const char *str;

#ifdef CONFIG_FWLOG_TTYMEM
	init_ttymem_console();
#endif
	debug("%s()\n", __func__);

	/*
	 * Now that pci_init initializes env device.
	 * Try to cleanup ethaddr env variables, this is needed
	 * as with each boot, configuration of QLM can change.
	 */
	cleanup_env_ethaddr();

	str = fdt_get_board_model();
	if (!str)
		str = "Marvell";
	snprintf(boardname, sizeof(boardname), "%s> ", str);
	env_set("prompt", boardname);
	set_working_fdt_addr(env_get_hex("fdtcontroladdr", fdt_base_addr));

	str = fdt_get_board_revision();
	if (str) {
		snprintf(boardrev, sizeof(boardrev), "%s", str);
		str = env_get("boardrev");
		if (str && strcmp(boardrev, str))
			save_env = true;
		env_set("boardrev", boardrev);
	}

	str = fdt_get_board_serial();
	if (str) {
		snprintf(boardserial, sizeof(boardserial), "%s", str);
		str = env_get("serial#");
		if (str && strcmp(boardserial, str))
			save_env = true;
		env_set("serial#", boardserial);
	}

	if (IS_ENABLED(CONFIG_CN20K_ETH_INTF))
		init_sh_fwdata();

	if (IS_ENABLED(CONFIG_NET_CN20K))
		probe_network_devices(true);

	if (IS_ENABLED(CONFIG_TARGET_CN20K_A))
		board_switch_init();

	if (save_env)
		env_save();

	return 0;
}

void board_quiesce_devices(void)
{
	struct uclass *uc;
	struct udevice *dev, *next;
	int ret;

	ret = uclass_get(UCLASS_ETH, &uc);
	if (!ret) {
		uclass_foreach_dev_safe(dev, next, uc) {
			device_remove(dev, DM_REMOVE_NORMAL);
		}
	}

	/* Bring down all lmac links */
	if (IS_ENABLED(CONFIG_CN20K_ETH_INTF))
		eth_intf_shutdown();

	/* Removes all RPM and RVU AF devices */
	if (IS_ENABLED(CONFIG_NET_CN20K))
		probe_network_devices(false);

	/* SMC call - removes all LF<->PF mappings */
	smc_disable_rvu_lfs(0);

#if CONFIG_IS_ENABLED(WDT)
	/* Stop watchdog */
	wdt_stop_all();
#endif
}

/*
 * Invoked before relocation, so limit to stack variables.
 */
int checkboard(void)
{
	const char *str;

	str = fdt_get_board_model();
	if (!str)
		str = "UNKNOWN";

	printf("Board: %s\n", str);

	return 0;
}

int board_acquire_flash_arb(bool acquire)
{
	union cpc_boot_ownerx ownerx;
	int ret = 0;

	if (!acquire) {
		ownerx.u = readl(CPC_BOOT_OWNERX(3));
		ownerx.s.boot_req = 0;
		writel(ownerx.u, CPC_BOOT_OWNERX(3));
	} else {
		ownerx.u = 0;
		ownerx.s.boot_req = 1;
		writel(ownerx.u, CPC_BOOT_OWNERX(3));
		udelay(1);
		ret = readl_relaxed_poll_timeout(CPC_BOOT_OWNERX(3), ownerx.u,
						 ((ownerx.s.boot_wait) == 0), 1000);
	}

	if (ret)
		debug("%s: Failed to acquire flash\n", __func__);

	return ret;
}

#ifdef CONFIG_LAST_STAGE_INIT
int last_stage_init(void)
{
#if CONFIG_IS_ENABLED(OCTEONTX_SERIAL_BOOTCMD)
	if (init_bootcmd_console())
		printf("Failed to init bootcmd input\n");
#endif

	(void)smc_flsf_fw_booted();

	return 0;
}
#endif
