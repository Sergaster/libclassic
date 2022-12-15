/*
   Copyright (C) 2022 Sergey V. Kostyuk

   This file is part of libclassic.
   Author: Sergey V. Kostyuk <kostyuk.sergey79@gmail.com>
   Algo: chained hash-table, with chains sorted by hash
   Ref:  [Gonnet 1984], [Knuth 1998].

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>. */

#ifndef CCL_HASHTABLE1_H
#define CCL_HASHTABLE1_H

#include <stdlib.h>

#include <classic/common.h>
#include <classic/map.h>

#ifdef  __cplusplus
extern "C" {
#endif

struct ccl_ht1_t;

typedef struct ccl_ht1_node_t {
	struct ccl_ht1_node_t *next;
	void *key;
	void *value;
	unsigned hash;
} ccl_ht1_node;

typedef struct ccl_ht1_t {
	ccl_ht1_node **table;
	ccl_cmp_cb cmp;
	ccl_free_cb kfree;
	ccl_free_cb vfree;
	ccl_hash_cb hash;
	size_t count;
	unsigned size;
} ccl_ht1;

ccl_ht1 *ccl_ht1_new(ccl_cmp_cb cmp_cb, ccl_free_cb kfree_cb, ccl_free_cb vfree_cb, ccl_hash_cb hash_cb, unsigned size);
void ccl_ht1_clear(ccl_ht1 *ht);
void ccl_ht1_free(ccl_ht1 *ht);
int ccl_ht1_select(ccl_ht1 *ht, void *k, void **v);
int ccl_ht1_insert(ccl_ht1 *ht, void *k, void *v, void **);
int ccl_ht1_unlink(ccl_ht1 *ht, void *key, void **k, void **v);
int ccl_ht1_delete(ccl_ht1 *ht, void *key);
int ccl_ht1_foreach(ccl_ht1 *ht, ccl_dforeach_cb cb, void *user);

ccl_map *ccl_map_ht1(ccl_cmp_cb cmp_cb, ccl_free_cb kfree_cb, ccl_free_cb vfree_cb, ccl_hash_cb hash_cb, unsigned size);

#ifdef  __cplusplus
}
#endif

#endif
