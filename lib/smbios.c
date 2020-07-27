// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * Adapted from coreboot src/arch/x86/smbios.c
 */

#include <common.h>
#include <dm.h>
#include <env.h>
#include <linux/stringify.h>
#include <mapmem.h>
#include <smbios.h>
#include <sysinfo.h>
#include <tables_csum.h>
#include <version.h>
#ifdef CONFIG_CPU
#include <cpu.h>
#include <dm/uclass-internal.h>
#endif
#include <linux/bitops.h>

/* cache details in needed in type 7 */
struct cache_details cache_data[NO_OF_CACHE] = {
	{"CACHE-D-L1", DMTF_TYPE7_CACHE_L1, DMTF_TYPE7_SINGLE_BIT_ECC, DMTF_TYPE7_2_WAY_SET_ASSOCIATIVE, DMTF_TYPE7_DATA_CACHE, DMTF_TYPE7_32K},
	{"CACHE-I-L1", DMTF_TYPE7_CACHE_L1,  DMTF_TYPE7_PARITY, DMTF_TYPE7_2_WAY_SET_ASSOCIATIVE, DMTF_TYPE7_INSTRUCTION_CACHE, DMTF_TYPE7_48K},
	{"CACHE-L2", DMTF_TYPE7_CACHE_L2, DMTF_TYPE7_SINGLE_BIT_ECC, DMTF_TYPE7_16_WAY_SET_ASSOCIATIVE, DMTF_TYPE7_DATA_UNIFIED, DMTF_TYPE7_512K},
};

/* Safeguard for checking that U_BOOT_VERSION_NUM macros are compatible with U_BOOT_DMI */
#if U_BOOT_VERSION_NUM < 2000 || U_BOOT_VERSION_NUM > 2099 || \
    U_BOOT_VERSION_NUM_PATCH < 1 || U_BOOT_VERSION_NUM_PATCH > 12
#error U_BOOT_VERSION_NUM macros are not compatible with DMI, fix U_BOOT_DMI macros
#endif

/*
 * U_BOOT_DMI_DATE contains BIOS Release Date in format mm/dd/yyyy.
 * BIOS Release Date is calculated from U-Boot version and fixed day 01.
 * So for U-Boot version 2021.04 it is calculated as "04/01/2021".
 * BIOS Release Date should contain date when code was released
 * and not when it was built or compiled.
 */
#if U_BOOT_VERSION_NUM_PATCH < 10
#define U_BOOT_DMI_MONTH "0" __stringify(U_BOOT_VERSION_NUM_PATCH)
#else
#define U_BOOT_DMI_MONTH __stringify(U_BOOT_VERSION_NUM_PATCH)
#endif
#define U_BOOT_DMI_DAY "01"
#define U_BOOT_DMI_YEAR __stringify(U_BOOT_VERSION_NUM)
#define U_BOOT_DMI_DATE U_BOOT_DMI_MONTH "/" U_BOOT_DMI_DAY "/" U_BOOT_DMI_YEAR

DECLARE_GLOBAL_DATA_PTR;

/**
 * struct smbios_ctx - context for writing SMBIOS tables
 *
 * @node:	node containing the information to write (ofnode_null() if none)
 * @dev:	sysinfo device to use (NULL if none)
 * @eos:	end-of-string pointer for the table being processed. This is set
 *		up when we start processing a table
 * @next_ptr:	pointer to the start of the next string to be added. When the
 *		table is nopt empty, this points to the byte after the \0 of the
 *		previous string.
 * @last_str:	points to the last string that was written to the table, or NULL
 *		if none
 */
struct smbios_ctx {
	ofnode node;
	struct udevice *dev;
	char *eos;
	char *next_ptr;
	char *last_str;
};

/**
 * Function prototype to write a specific type of SMBIOS structure
 *
 * @addr:	start address to write the structure
 * @handle:	the structure's handle, a unique 16-bit number
 * @ctx:	context for writing the tables
 * Return:	size of the structure
 */
typedef int (*smbios_write_type)(ulong *addr, int handle,
				 struct smbios_ctx *ctx);

/**
 * struct smbios_write_method - Information about a table-writing function
 *
 * @write: Function to call
 * @subnode_name: Name of subnode which has the information for this function,
 *	NULL if none
 */
struct smbios_write_method {
	smbios_write_type write;
	const char *subnode_name;
};

struct smbios_type8 type8_data[3] = {
	{
		0, 0, 0, 0, 0xFF, 0,  DMTF_TYPE8_RJ45, DMTF_TYPE8_NETWORK_PORT
	},

	{
		0, 0, 0, 0, DMTF_TYPE8_ACCESS_BUS, 0,  DMTF_TYPE8_ACCESS_BUS, DMTF_TYPE8_USB
	},

	{
		0, 0, 0, 0, DMTF_TYPE8_ACCESS_BUS, 0,  DMTF_TYPE8_ACCESS_BUS, DMTF_TYPE8_USB
	},
};

struct type9_data plat_type9_data[MAX_SLOTS] = {
	{"PCIe Slot 1", DMTF_TYPE9_SLOT_TYPE_PCIE, DMTF_TYPE9_SLOT_DATA_BUS_WIDTH_X4, DMTF_TYPE9_CURRENT_USAGE_AVAILABLE, DMTF_TYPE9_SLOT_LENGTH_LONG, 1, DMTF_TYPE9_SLOT_CHAR_1_3_3V},
	{"PCIe Slot 2", DMTF_TYPE9_SLOT_TYPE_PCIE, DMTF_TYPE9_SLOT_DATA_BUS_WIDTH_X4, DMTF_TYPE9_CURRENT_USAGE_AVAILABLE, DMTF_TYPE9_SLOT_LENGTH_LONG, 2, DMTF_TYPE9_SLOT_CHAR_1_3_3V},
};

/**
 * smbios_add_string() - add a string to the string area
 *
 * This adds a string to the string area which is appended directly after
 * the formatted portion of an SMBIOS structure.
 *
 * @ctx:	SMBIOS context
 * @str:	string to add
 * Return:	string number in the string area (1 or more)
 */
static int smbios_add_string(struct smbios_ctx *ctx, const char *str)
{
	int i = 1;
	char *p = ctx->eos;

	if (!*str)
		str = "Unknown";

	for (;;) {
		if (!*p) {
			ctx->last_str = p;
			strcpy(p, str);
			p += strlen(str);
			*p++ = '\0';
			ctx->next_ptr = p;
			*p++ = '\0';

			return i;
		}

		if (!strcmp(p, str)) {
			ctx->last_str = p;
			return i;
		}

		p += strlen(p) + 1;
		i++;
	}
}

/**
 * smbios_add_prop_si() - Add a property from the devicetree or sysinfo
 *
 * Sysinfo is used if available, with a fallback to devicetree
 *
 * @ctx:	context for writing the tables
 * @prop:	property to write
 * Return:	0 if not found, else SMBIOS string number (1 or more)
 */
static int smbios_add_prop_si(struct smbios_ctx *ctx, const char *prop,
			      int sysinfo_id)
{
	if (sysinfo_id && ctx->dev) {
		char val[SMBIOS_STR_MAX];
		int ret;

		ret = sysinfo_get_str(ctx->dev, sysinfo_id, sizeof(val), val);
		if (!ret)
			return smbios_add_string(ctx, val);
	}
	if (IS_ENABLED(CONFIG_OF_CONTROL)) {
		const char *str;

		str = ofnode_read_string(ctx->node, prop);
		if (str)
			return smbios_add_string(ctx, str);
	}

	return 0;
}

/**
 * smbios_add_prop() - Add a property from the devicetree
 *
 * @prop:	property to write
 * Return:	0 if not found, else SMBIOS string number (1 or more)
 */
static int smbios_add_prop(struct smbios_ctx *ctx, const char *prop)
{
	return smbios_add_prop_si(ctx, prop, SYSINFO_ID_NONE);
}

static void smbios_set_eos(struct smbios_ctx *ctx, char *eos)
{
	ctx->eos = eos;
	ctx->next_ptr = eos;
	ctx->last_str = NULL;
}

int smbios_update_version(const char *version)
{
	char *ptr = gd->smbios_version;
	uint old_len, len;

	if (!ptr)
		return log_ret(-ENOENT);

	/*
	 * This string is supposed to have at least enough bytes and is
	 * padded with spaces. Update it, taking care not to move the
	 * \0 terminator, so that other strings in the string table
	 * are not disturbed. See smbios_add_string()
	 */
	old_len = strnlen(ptr, SMBIOS_STR_MAX);
	len = strnlen(version, SMBIOS_STR_MAX);
	if (len > old_len)
		return log_ret(-ENOSPC);

	log_debug("Replacing SMBIOS type 0 version string '%s'\n", ptr);
	memcpy(ptr, version, len);
#ifdef LOG_DEBUG
	print_buffer((ulong)ptr, ptr, 1, old_len + 1, 0);
#endif

	return 0;
}

/**
 * smbios_string_table_len() - compute the string area size
 *
 * This computes the size of the string area including the string terminator.
 *
 * @ctx:	SMBIOS context
 * Return:	string area size
 */
static int smbios_string_table_len(const struct smbios_ctx *ctx)
{
	/* Allow for the final \0 after all strings */
	return (ctx->next_ptr + 1) - ctx->eos;
}

static int smbios_write_type0(ulong *current, int handle,
			      struct smbios_ctx *ctx)
{
	struct smbios_type0 *t;
	int len = sizeof(struct smbios_type0);

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type0));
	fill_smbios_header(t, SMBIOS_BIOS_INFORMATION, len, handle);
	smbios_set_eos(ctx, t->eos);
	t->vendor = smbios_add_string(ctx, "U-Boot");

	t->bios_ver = smbios_add_prop(ctx, "version");
	if (!t->bios_ver)
		t->bios_ver = smbios_add_string(ctx, PLAIN_VERSION);
	if (t->bios_ver)
		gd->smbios_version = ctx->last_str;
	log_debug("smbios_version = %p: '%s'\n", gd->smbios_version,
		  gd->smbios_version);
#ifdef LOG_DEBUG
	print_buffer((ulong)gd->smbios_version, gd->smbios_version,
		     1, strlen(gd->smbios_version) + 1, 0);
#endif
	t->bios_release_date = smbios_add_string(ctx, U_BOOT_DMI_DATE);
#ifdef CONFIG_ROM_SIZE
	t->bios_rom_size = (CONFIG_ROM_SIZE / 65536) - 1;
#endif
	t->bios_characteristics = BIOS_CHARACTERISTICS_PCI_SUPPORTED |
				  BIOS_CHARACTERISTICS_SELECTABLE_BOOT |
				  BIOS_CHARACTERISTICS_UPGRADEABLE;
#ifdef CONFIG_GENERATE_ACPI_TABLE
	t->bios_characteristics_ext1 = BIOS_CHARACTERISTICS_EXT1_ACPI;
#endif
#ifdef CONFIG_EFI_LOADER
	t->bios_characteristics_ext2 |= BIOS_CHARACTERISTICS_EXT2_UEFI;
#endif
	t->bios_characteristics_ext2 |= BIOS_CHARACTERISTICS_EXT2_TARGET;

	/* bios_major_release has only one byte, so drop century */
	t->bios_major_release = U_BOOT_VERSION_NUM % 100;
	t->bios_minor_release = U_BOOT_VERSION_NUM_PATCH;
	t->ec_major_release = 0xff;
	t->ec_minor_release = 0xff;

	len = t->length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type1(ulong *current, int handle,
			      struct smbios_ctx *ctx)
{
	struct smbios_type1 *t;
	int len = sizeof(struct smbios_type1);
	char *serial_str = env_get("serial#");

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type1));
	fill_smbios_header(t, SMBIOS_SYSTEM_INFORMATION, len, handle);
	smbios_set_eos(ctx, t->eos);
	t->manufacturer = smbios_add_prop(ctx, "manufacturer");
	if (!t->manufacturer)
		t->manufacturer = smbios_add_string(ctx, "Unknown");
	t->product_name = smbios_add_prop(ctx, "product");
	if (!t->product_name)
		t->product_name = smbios_add_string(ctx, "Unknown Product");
	t->version = smbios_add_prop_si(ctx, "version",
					SYSINFO_ID_SMBIOS_SYSTEM_VERSION);
	if (serial_str) {
		t->serial_number = smbios_add_string(ctx, serial_str);
		strncpy((char *)t->uuid, serial_str, sizeof(t->uuid));
	} else {
		t->serial_number = smbios_add_prop(ctx, "serial");
	}
	t->sku_number = smbios_add_prop(ctx, "sku");
	t->family = smbios_add_prop(ctx, "family");

	len = t->length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type2(ulong *current, int handle,
			      struct smbios_ctx *ctx)
{
	struct smbios_type2 *t;
	int len = sizeof(struct smbios_type2);

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type2));
	fill_smbios_header(t, SMBIOS_BOARD_INFORMATION, len, handle);
	smbios_set_eos(ctx, t->eos);
	t->manufacturer = smbios_add_prop(ctx, "manufacturer");
	if (!t->manufacturer)
		t->manufacturer = smbios_add_string(ctx, "Unknown");
	t->product_name = smbios_add_prop(ctx, "product");
	if (!t->product_name)
		t->product_name = smbios_add_string(ctx, "Unknown Product");
	t->version = smbios_add_prop_si(ctx, "version",
					SYSINFO_ID_SMBIOS_BASEBOARD_VERSION);
	t->asset_tag_number = smbios_add_prop(ctx, "asset-tag");
	t->feature_flags = SMBIOS_BOARD_FEATURE_HOSTING;
	t->board_type = SMBIOS_BOARD_MOTHERBOARD;

	len = t->length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type3(ulong *current, int handle,
			      struct smbios_ctx *ctx)
{
	struct smbios_type3 *t;
	int len = sizeof(struct smbios_type3);

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type3));
	fill_smbios_header(t, SMBIOS_SYSTEM_ENCLOSURE, len, handle);
	smbios_set_eos(ctx, t->eos);
	t->manufacturer = smbios_add_prop(ctx, "manufacturer");
	if (!t->manufacturer)
		t->manufacturer = smbios_add_string(ctx, "Unknown");
	t->chassis_type = SMBIOS_ENCLOSURE_DESKTOP;
	t->bootup_state = SMBIOS_STATE_SAFE;
	t->power_supply_state = SMBIOS_STATE_SAFE;
	t->thermal_state = SMBIOS_STATE_SAFE;
	t->security_status = SMBIOS_SECURITY_NONE;

	len = t->length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static void smbios_write_type4_dm(struct smbios_type4 *t,
				  struct smbios_ctx *ctx)
{
	u8 proc_type = 0;
	u8 voltage = 0;
	u8 core_count = 0;
	u8 core_enabled = 0;
	u8 thread_count = 0;
	u16 max_speed = 0;
	u16 proc_char = 0;
	u8 status = 0;
	u16 processor_family = SMBIOS_PROCESSOR_FAMILY_UNKNOWN;
	const char *vendor = "Unknown";
	const char *name = "Unknown";
	const char *serial_number = "Unknown";
	const char *asset_tag = "Unknown";
	const char *part_number = "Unknown";
	const char *socket_designation = "Unknown";

#ifdef CONFIG_CPU
	char processor_name[49];
	char vendor_name[49];
	struct udevice *cpu = NULL;
	struct cpu_info info;

	uclass_find_first_device(UCLASS_CPU, &cpu);
	if (cpu) {
		struct cpu_plat *plat = dev_get_parent_plat(cpu);

		cpu_get_info(cpu, &info);
		processor_family = is_proc_arm(info.cpu_type);
		if (processor_family) {
			t->processor_family = DMTF_CHECK_FAMILY2;
			t->processor_family2 = processor_family;
		} else {
			t->processor_family = processor_family;
		}

		t->processor_id[0] = plat->id[0];
		t->processor_id[1] = plat->id[1];

		if (!cpu_get_vendor(cpu, vendor_name, sizeof(vendor_name)))
			vendor = vendor_name;
		if (!cpu_get_desc(cpu, processor_name, sizeof(processor_name)))
			name = processor_name;

		core_count = cpu_get_count(dev);
		status = plat->status;
		proc_type = plat->proc_type;
		voltage = plat->voltage;
		core_enabled = plat->core_enabled;
		thread_count = plat->thread_count;
		max_speed = plat->max_speed;
		proc_char = plat->proc_char;
		serial_number = plat->serial_number;
		asset_tag = plat->asset_tag;
		part_number = plat->part_number;
		socket_designation = plat->socket_designation;
	}
#endif

	t->processor_family = processor_family;
	t->processor_manufacturer = smbios_add_string(ctx, vendor);
	t->processor_version = smbios_add_string(ctx, name);
	t->status =  status;
	t->processor_type = proc_type;
	t->voltage = voltage;
	t->core_count = core_count;
	t->core_enabled = core_enabled;
	t->thread_count = thread_count;
	t->max_speed = max_speed;
	t->processor_characteristics = proc_char;
	t->serial_number = smbios_add_string(ctx, serial_number);
	t->asset_tag = smbios_add_string(ctx, asset_tag);
	t->part_number = smbios_add_string(ctx, part_number);
	t->socket_designation = smbios_add_string(ctx, socket_designation);
}

static int smbios_write_type4(ulong *current, int handle,
			      struct smbios_ctx *ctx)
{
	struct smbios_type4 *t;
	int len = sizeof(struct smbios_type4);

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type4));
	fill_smbios_header(t, SMBIOS_PROCESSOR_INFORMATION, len, handle);
	smbios_set_eos(ctx, t->eos);
	t->processor_type = SMBIOS_PROCESSOR_TYPE_CENTRAL;
	smbios_write_type4_dm(t, ctx);
	t->status = SMBIOS_PROCESSOR_STATUS_ENABLED;

	t->l1_cache_handle = 0x500;
	t->l2_cache_handle = 0x502;
	t->l3_cache_handle = 0xFFFF;
	t->processor_upgrade = SMBIOS_PROCESSOR_UPGRADE_NONE;

	len = t->length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type7_dm(ulong *current, int handle, int index)
{
	struct smbios_type7 *t;
	int len = sizeof(struct smbios_type7);

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type7));
	fill_smbios_header(t, SMBIOS_CACHE_INFORMATION, len, handle + index);

	t->socket_designation = smbios_add_string(t->eos, cache_data[index].name);
	t->error_correction_type = cache_data[index].error_control;
	t->system_cache_type = cache_data[index].type;
	t->associativity = cache_data[index].associativity;
	t->maximum_cache_size2 = cache_data[index].size;
	t->installed_cache_size2 = cache_data[index].size;
	t->cache_configuration = DMTF_TYPE7_CACHE_ENABLED |
							cache_data[index].level | DMTF_TYPE7_OP_MODE;

	t->supported_sram_type = DMTF_TYPE7_SRAM_TYPE_UNKNOWN;
	t->current_sram_type = DMTF_TYPE7_SRAM_TYPE_UNKNOWN;

	len = t->length + smbios_string_table_len(t->eos);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type7(ulong *current, int handle)
{
	u32 i = 0, len = 0;

	for (; i < NO_OF_CACHE; i++)
		len += smbios_write_type7_dm(current, handle, i);

	return len;
}

static int smbios_write_type8_dm(ulong *current, int handle, int index)
{
	struct smbios_type8 *t;
	int len = sizeof(struct smbios_type8);

	char type8_reference_designator[MAX_PORTS][10] = {
		"RJ45", "USB 1", "USB 2"
	};

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type8));
	fill_smbios_header(t, SMBIOS_PORT_INFORMATION, len, handle + index);

	t->internal_reference_designator = smbios_add_string(t->eos, type8_reference_designator[index]);
	t->external_reference_designator = smbios_add_string(t->eos, type8_reference_designator[index]);

	t->internal_connector_type = type8_data[index].internal_connector_type;
	t->external_connector_type = type8_data[index].external_connector_type;
	t->port_type = type8_data[index].port_type;

	len = t->length + smbios_string_table_len(t->eos);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type8(ulong *current, int handle)
{
	u32 no_of_handles = MAX_PORTS, i = 0, len = 0;

	for (; i < no_of_handles; i++)
		len += smbios_write_type8_dm(current, handle, i);

	return len;
}

static int smbios_write_type9_dm(ulong *current, int handle, int index)
{
	struct smbios_type9 *t;
	int len = sizeof(struct smbios_type9);

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type9));
	fill_smbios_header(t, SMBIOS_SYSTEM_SLOTS, len, handle + index);

	t->slot_designation = smbios_add_string(t->eos, plat_type9_data[index].slot_designation);

	t->slot_id = plat_type9_data[index].slot_id;
	t->slot_length = plat_type9_data[index].slot_length;
	t->slot_type = plat_type9_data[index].slot_type;
	t->slot_data_bus_width = plat_type9_data[index].slot_data_bus_width;

	t->current_usage = plat_type9_data[index].current_usage;
	t->slot_characteristics_1 = plat_type9_data[index].slot_characteristics_1;

	len = t->length + smbios_string_table_len(t->eos);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type9(ulong *current, int handle)
{
	u32 no_of_handles = MAX_SLOTS, i = 0, len = 0;

	for (; i < no_of_handles; i++)
		len += smbios_write_type9_dm(current, handle, i);

	return len;
}

static int smbios_write_type13(ulong *current, int handle)
{
	struct smbios_type13 *t;
	int len = sizeof(struct smbios_type13);
	const char *language = "en|US|iso8859-1";

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type13));
	fill_smbios_header(t, SMBIOS_BIOS_LANGUAGE_INFORMATION, len, handle);

	t->installable_languages = 1;
	t->flags = 0;
	t->current_language = smbios_add_string(t->eos, language);

	len = t->length + smbios_string_table_len(t->eos);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type16(ulong *current, int handle)
{
	struct smbios_type16 *t;
	int len = sizeof(struct smbios_type16);

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type16));
	fill_smbios_header(t, SMBIOS_PHYS_MEMORY_ARRAY, len, handle);

	t->location = DMTF_TYPE16_LOCATION_SYSTEM_BOARD_OR_MOTHERBOARD;
	t->use = DMTF_TYPE16_USE_SYSTEM_MEMORY;
	t->memory_error_correction = DMTF_TYPE16_ERROR_CORRECTION_SINGLE_BIT_ECC;
	t->maximum_capacity = DMTF_TYPE16_MAXIMUM_CAPACITY_64GB;
	t->number_of_memory_devices = 1;
	t->memory_error_information_handle = 0xFFFE;

	len = t->length + smbios_string_table_len(t->eos);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type17_dm(ulong *current, int handle, int index)
{
	struct smbios_type17 *t;
	int len = sizeof(struct smbios_type17);
	const char *manufacturer = "UNKNOWN";

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type17));
	fill_smbios_header(t, SMBIOS_MEMORY_DEVICE, len, handle + index);

	/* Data to be filled by OEM */
	/* ...
	 * ...
	 * ...
	 */
	t->physical_memory_array_handle = 0x0800;
	t->size = DMTF_TYPE17_4GB;
	t->form_factor = DMTF_TYPE17_DIMM;
	t->manufacturer = smbios_add_string(t->eos, manufacturer);

	len = t->length + smbios_string_table_len(t->eos);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type17(ulong *current, int handle)
{
	u32 no_of_handles = MAX_MEMORY_DEV, i = 0, len = 0;

	for (; i < no_of_handles; i++)
		len += smbios_write_type17_dm(current, handle, i);

	return len;
}

static int smbios_write_type32(ulong *current, int handle,
			       struct smbios_ctx *ctx)
{
	struct smbios_type32 *t;
	int len = sizeof(struct smbios_type32);

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type32));
	fill_smbios_header(t, SMBIOS_SYSTEM_BOOT_INFORMATION, len, handle);
	smbios_set_eos(ctx, t->eos);

	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type127(ulong *current, int handle,
				struct smbios_ctx *ctx)
{
	struct smbios_type127 *t;
	int len = sizeof(struct smbios_type127);

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type127));
	fill_smbios_header(t, SMBIOS_END_OF_TABLE, len, handle);

	*current += len;
	unmap_sysmem(t);

	return len;
}

static struct smbios_write_method smbios_write_funcs[] = {
	{ smbios_write_type0, "bios", },
	{ smbios_write_type1, "system", },
	{ smbios_write_type2, "baseboard", },
	{ smbios_write_type3, "chassis", },
	{ smbios_write_type4, },
	{ smbios_write_type7, },
	{ smbios_write_type8, },
	{ smbios_write_type9, },
	{ smbios_write_type13, },
	{ smbios_write_type16, },
	{ smbios_write_type17, },
	{ smbios_write_type32, },
	{ smbios_write_type127 },
};

ulong write_smbios_table(ulong addr)
{
	ofnode parent_node = ofnode_null();
	struct smbios_entry *se;
	struct smbios_ctx ctx;
	ulong table_addr;
	ulong tables;
	int len = 0;
	int max_struct_size = 0;
	int handle = 0;
	char *istart;
	int isize;
	int i;

	ctx.node = ofnode_null();
	if (IS_ENABLED(CONFIG_OF_CONTROL)) {
		uclass_first_device(UCLASS_SYSINFO, &ctx.dev);
		if (ctx.dev)
			parent_node = dev_read_subnode(ctx.dev, "smbios");
	} else {
		ctx.dev = NULL;
	}

	/* 16 byte align the table address */
	addr = ALIGN(addr, 16);

	se = map_sysmem(addr, sizeof(struct smbios_entry));
	memset(se, 0, sizeof(struct smbios_entry));

	addr += sizeof(struct smbios_entry);
	addr = ALIGN(addr, 16);
	tables = addr;

	/* populate minimum required tables */
	for (i = 0; i < ARRAY_SIZE(smbios_write_funcs); i++) {
		const struct smbios_write_method *method;
		int tmp;

		method = &smbios_write_funcs[i];
		if (IS_ENABLED(CONFIG_OF_CONTROL) && method->subnode_name)
			ctx.node = ofnode_find_subnode(parent_node,
						       method->subnode_name);
		tmp = method->write((ulong *)&addr, handle, &ctx);

		max_struct_size = max(max_struct_size, tmp);
		len += tmp;
		handle += 0x100;
	}

	memcpy(se->anchor, "_SM_", 4);
	se->length = sizeof(struct smbios_entry);
	se->major_ver = SMBIOS_MAJOR_VER;
	se->minor_ver = SMBIOS_MINOR_VER;
	se->max_struct_size = max_struct_size;
	memcpy(se->intermediate_anchor, "_DMI_", 5);
	se->struct_table_length = len;

	/*
	 * We must use a pointer here so things work correctly on sandbox. The
	 * user of this table is not aware of the mapping of addresses to
	 * sandbox's DRAM buffer.
	 */
	table_addr = (ulong)map_sysmem(tables, 0);
	if (sizeof(table_addr) > sizeof(u32) && table_addr > (ulong)UINT_MAX) {
		/*
		 * We need to put this >32-bit pointer into the table but the
		 * field is only 32 bits wide.
		 */
		printf("WARNING: SMBIOS table_address overflow %llx\n",
		       (unsigned long long)table_addr);
		addr = 0;
		goto out;
	}
	se->struct_table_address = table_addr;

	se->struct_count = handle;

	/* calculate checksums */
	istart = (char *)se + SMBIOS_INTERMEDIATE_OFFSET;
	isize = sizeof(struct smbios_entry) - SMBIOS_INTERMEDIATE_OFFSET;
	se->intermediate_checksum = table_compute_checksum(istart, isize);
	se->checksum = table_compute_checksum(se, sizeof(struct smbios_entry));
out:
	unmap_sysmem(se);

	return addr;
}
