/**
 * (C) Copyright 2014, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
**/
#ifndef __ATF_H__
#define __ATF_H__

ssize_t atf_dram_size(unsigned int node);
ssize_t atf_node_count(void);
ssize_t atf_disable_rvu_lfs(unsigned int node);
#endif
