/** @file
#
#  Copyright (c) 2014, Cavium Inc. All rights reserved.<BR>
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
#**/

/* #define DEBUG */

#include <common.h>

#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <memalign.h>

#include <asm/arch/atf.h>
#include <asm/arch/atf_part.h>
#include <asm/arch/thunderx_svc.h>

#include <search.h>
#include <errno.h>

#if defined(CONFIG_ENV_SIZE_REDUND) &&  \
	(CONFIG_ENV_SIZE_REDUND != CONFIG_ENV_SIZE)
#error CONFIG_ENV_SIZE_REDUND should be the same as CONFIG_ENV_SIZE
#endif

char *env_name_spec = "ATF";

#ifdef ENV_IS_EMBEDDED
env_t *env_ptr = &environment;
#else /* ! ENV_IS_EMBEDDED */
env_t *env_ptr;
#endif /* ENV_IS_EMBEDDED */

DECLARE_GLOBAL_DATA_PTR;

#if !defined(CONFIG_ENV_OFFSET)
#define CONFIG_ENV_OFFSET 0
#endif

__weak int atf_get_env_addr(int copy, ulong *env_addr)
{
	s64 offset;

/* TODO: Create a special partition */
	offset = CONFIG_ENV_OFFSET;
#ifdef CONFIG_ENV_OFFSET_REDUND
	if (copy)
		offset = CONFIG_ENV_OFFSET_REDUND;
#endif

#if 0
	if (offset < 0)
#endif

	*env_addr = offset;

	return 0;
}

int env_init(void)
{
	/* use default */
	gd->env_addr	= (ulong)&default_environment[0];
	gd->env_valid	= 1;

	return 0;
}

#ifdef CONFIG_CMD_SAVEENV

static inline int write_env(unsigned long offset, const void *buffer, size_t size)
{
	/* FIXME: Remove this when ATF gets a decent MMU setup */
	void *lowmem_buff = (void *)CONFIG_SYS_LOWMEM_BASE;
	int ret;

	memcpy(lowmem_buff, buffer, size);
#if defined(CONFIG_SYS_ENV_ATF_MMC)
	ret = atf_write_mmc(offset, lowmem_buff, size);
#elif defined(CONFIG_SYS_ENV_ATF_NOR)
	ret = atf_erase_nor(offset, size);
	if (ret < 0) {
		return ret;
	}
	ret = atf_write_nor(offset, lowmem_buff, size);
	if (ret < 0) {
		return ret;
	}
#else
#error "Unknown storage facility"
#endif

	return 0;
}

#ifdef CONFIG_ENV_OFFSET_REDUND
static unsigned char env_flags;
#endif

int saveenv(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(env_t, env_new, 1);
	u32	offset;
	int	ret, copy = 0;

	ret = env_export(env_new);
	if (ret)
		goto err;

#ifdef CONFIG_ENV_OFFSET_REDUND
	env_new->flags	= ++env_flags; /* increase the serial */

	if (gd->env_valid == 1)
		copy = 1;
#endif

	if (atf_get_env_addr(copy, (ulong *)&offset)) {
		ret = 1;
		goto err;
	}

	printf("Writing to %s environment block... ", copy ? "redundant " : "");

	if (write_env(offset, (u_char *)env_new, CONFIG_ENV_SIZE)) {
		puts("failed\n");
		ret = 1;
		goto err;
	}

	puts("done\n");
	ret = 0;

#ifdef CONFIG_ENV_OFFSET_REDUND
	gd->env_valid = gd->env_valid == 2 ? 1 : 2;
#endif

err:
	return ret;
}
#endif /* CONFIG_CMD_SAVEENV */

static inline int read_env(unsigned long offset, void *buffer, unsigned long size)
{
	/* FIXME: Remove this when ATF gets a decent MMU setup */
	void *lowmem_buff = (void *)CONFIG_SYS_LOWMEM_BASE;
	int ret;

#if defined(CONFIG_SYS_ENV_ATF_MMC)
	ret = atf_read_mmc(offset, lowmem_buff, size);
#elif defined(CONFIG_SYS_ENV_ATF_NOR)
	ret = atf_read_nor(offset, lowmem_buff, size);
#else
#error "Unknown storage facility"
#endif

	memcpy(buffer, lowmem_buff, size);

	return (ret < 0) ? ret : 0;
}

#ifdef CONFIG_ENV_OFFSET_REDUND
void env_relocate_spec(void)
{
#if !defined(ENV_IS_EMBEDDED)
	ulong offset1, offset2;
	int read1_fail = 0, read2_fail = 0;
	int crc1_ok = 0, crc2_ok = 0;
	env_t *ep;
	int ret;

	ALLOC_CACHE_ALIGN_BUFFER(env_t, tmp_env1, 1);
	ALLOC_CACHE_ALIGN_BUFFER(env_t, tmp_env2, 1);

	if (tmp_env1 == NULL || tmp_env2 == NULL) {
		puts("Can't allocate buffers for environment\n");
		ret = 1;
		goto err;
	}

	if (atf_get_env_addr(0, &offset1) ||
		atf_get_env_addr(1, &offset2)) {
		ret = 1;
		goto err;
	}

	read1_fail = read_env(offset1, tmp_env1, CONFIG_ENV_SIZE);
	read2_fail = read_env(offset2, tmp_env2, CONFIG_ENV_SIZE);

	if (read1_fail && read2_fail)
		puts("*** Error - No Valid Environment Area found\n");
	else if (read1_fail || read2_fail)
		puts("*** Warning - some problems detected "
		     "reading environment; recovered successfully\n");

	crc1_ok = !read1_fail &&
		(crc32(0, tmp_env1->data, ENV_SIZE) == tmp_env1->crc);
	crc2_ok = !read2_fail &&
		(crc32(0, tmp_env2->data, ENV_SIZE) == tmp_env2->crc);

	if (!crc1_ok && !crc2_ok) {
		ret = 1;
		goto err;
	} else if (crc1_ok && !crc2_ok) {
		gd->env_valid = 1;
	} else if (!crc1_ok && crc2_ok) {
		gd->env_valid = 2;
	} else {
		/* both ok - check serial */
		if (tmp_env1->flags == 255 && tmp_env2->flags == 0)
			gd->env_valid = 2;
		else if (tmp_env2->flags == 255 && tmp_env1->flags == 0)
			gd->env_valid = 1;
		else if (tmp_env1->flags > tmp_env2->flags)
			gd->env_valid = 1;
		else if (tmp_env2->flags > tmp_env1->flags)
			gd->env_valid = 2;
		else /* flags are equal - almost impossible */
			gd->env_valid = 1;
	}

	free(env_ptr);

	if (gd->env_valid == 1)
		ep = tmp_env1;
	else
		ep = tmp_env2;

	env_flags = ep->flags;
	env_import((char *)ep, 0);
	ret = 0;

err:
	if (ret)
		set_default_env(NULL);

#endif
}
#else /* ! CONFIG_ENV_OFFSET_REDUND */
void env_relocate_spec(void)
{
#if !defined(ENV_IS_EMBEDDED)
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE);
	ulong offset;
	int ret;

	if (atf_get_env_addr(0, &offset)) {
		ret = 1;
		goto err;
	}

	if (read_env(offset, buf, CONFIG_ENV_SIZE)) {
		ret = 1;
		goto err;
	}

	env_import(buf, 1);
	ret = 0;

err:
	if (ret)
		set_default_env(NULL);
#endif
}
#endif /* CONFIG_ENV_OFFSET_REDUND */
