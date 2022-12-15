/*
   Copyright (C) 2022 Sergey V. Kostyuk

   This file is part of libclassic.
   Author: Sergey V. Kostyuk <kostyuk.sergey79@gmail.com>
   Algo: open-addressing hash-table interface.
   Ref: [Gonnet 1984], [Knuth 1998].

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

#ifndef CCL_HASHTABLE2_H
#define CCL_HASHTABLE2_H

#include <stdlib.h>

#include <classic/common.h>
#include <classic/map.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct ccl_ht2_node_t {
	void *key;
	void *value;
	unsigned hash;
} ccl_ht2_node;

typedef struct ccl_ht2_t {
	ccl_ht2_node *table;
	ccl_cmp_cb cmp;
	ccl_free_cb kfree;
	ccl_free_cb vfree;
	ccl_hash_cb hash;
	size_t count;
	unsigned size;
} ccl_ht2;

ccl_ht2 *ccl_ht2_new(ccl_cmp_cb cmp_cb, ccl_free_cb kfree_cb, ccl_free_cb vfree_cb, ccl_hash_cb hash_cb, unsigned int size);
void ccl_ht2_clear(ccl_ht2 *ht);
void ccl_ht2_free(ccl_ht2 *ht);
int ccl_ht2_select(ccl_ht2 *ht, void *k, void **v);
int ccl_ht2_insert(ccl_ht2 *ht, void *k, void *v, void **);
int ccl_ht2_unlink(ccl_ht2 *ht, void *key, void **k, void **v);
int ccl_ht2_delete(ccl_ht2 *ht, void *key);
int ccl_ht2_foreach(ccl_ht2 *ht, ccl_dforeach_cb cb, void *user);

ccl_map *ccl_map_ht2(ccl_cmp_cb cmp_cb, ccl_free_cb kfree_cb, ccl_free_cb vfree_cb, ccl_hash_cb hash_cb, unsigned size);

#ifdef  __cplusplus
}
#endif

#endif
