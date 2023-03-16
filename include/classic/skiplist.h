/*
   Copyright (C) 2022 Sergey V. Kostyuk

   This file is part of libclassic.
   Author: Sergey V. Kostyuk <kostyuk.sergey79@gmail.com>
   Algo: skiplist implementation.
   Ref: [Pugh 1990], [Sedgewick 1998].

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

#ifndef CCL_SKIPLIST_H
#define CCL_SKIPLIST_H

#include <stdlib.h>

#include <classic/common.h>
#include <classic/map.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct ccl_skipnode_t {
	void *key;
	void *value;
	struct ccl_skipnode_t *prev;
	unsigned link_count;
	struct ccl_skipnode_t *link[];
} ccl_skipnode;

#define MAX_LINK		32

struct ccl_skiplist_t;

typedef unsigned (* ccl_maxlink_cb)(struct ccl_skiplist_t *);

typedef struct ccl_skiplist_t {
	struct ccl_skipnode_t *head;
	ccl_cmp_cb cmp;
	ccl_free_cb kfree;
	ccl_free_cb vfree;
	ccl_maxlink_cb maxlink;
	unsigned max_link;
	unsigned top_link;
	size_t count;
} ccl_skiplist;

typedef struct ccl_skiplist_iter_t {
	struct ccl_skipnode_t *node;
	struct ccl_skiplist_t *tree;
} ccl_skiplist_iter;

ccl_skiplist *ccl_skiplist_new(ccl_cmp_cb, ccl_free_cb, ccl_free_cb, ccl_maxlink_cb, unsigned);
size_t ccl_skiplist_clear(ccl_skiplist *tree);
void ccl_skiplist_free(ccl_skiplist *tree);
bool ccl_skiplist_select(ccl_skiplist *tree, void *k, void **v);
bool ccl_skiplist_insert(ccl_skiplist *tree, void *k, void *v, void **);
bool ccl_skiplist_unlink(ccl_skiplist *tree, void *key, void **k, void **v);
bool ccl_skiplist_delete(ccl_skiplist *tree, void *k);
bool ccl_skiplist_foreach(ccl_skiplist *tree, ccl_dforeach_cb cb, void *user);

/* sorted map */
ccl_map *ccl_smap_skiplist(ccl_cmp_cb, ccl_free_cb, ccl_free_cb, ccl_maxlink_cb, unsigned);

#ifdef  __cplusplus
}
#endif

#endif
