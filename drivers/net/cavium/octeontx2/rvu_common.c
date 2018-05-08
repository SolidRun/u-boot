/*
 * Copyright (C) 2018 Cavium, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include <common.h>
#include <net.h>
#include <netdev.h>
#include <malloc.h>
#include <dm.h>
#include <misc.h>
#include <asm/io.h>
#include <errno.h>

#include "rvu.h"
#include "rvu_common.h"

int rvu_alloc_bitmap(struct rsrc_bmap *rsrc)
{
	size_t size = (rsrc->max + sizeof(rsrc->bmap) * 8 - 1);

	rsrc->bmap = calloc(size / (sizeof(rsrc->bmap) * 8), 1);
	if (!rsrc->bmap)
		return -ENOMEM;
	return 0;
}

int qmem_alloc(struct qmem *q, u32 qsize, size_t entry_sz)
{
	q->base = memalign(CONFIG_SYS_CACHELINE_SIZE, qsize * entry_sz);
	if (!q->base)
		return -ENOMEM;
	q->entry_sz = entry_sz;
	q->qsize = qsize;
	q->alloc_sz = qsize * entry_sz;
	q->iova = (dma_addr_t)(q->base);
	return 0;
}

void qmem_free(struct qmem *q)
{
	if (q->base)
		free(q->base);
	memset(q, 0, sizeof(*q));
}
