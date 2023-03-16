/*
   Copyright (C) 2022 Sergey V. Kostyuk

   This file is part of libclassic.
   Author: Sergey V. Kostyuk <kostyuk.sergey79@gmail.com>
   Algo: treap tree.
   Ref: [Aragon and Seidel, 1996], [Knuth 1998].

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

#ifndef CCL_TR_TREE_H
#define CCL_TR_TREE_H

#include <stdlib.h>
#include <stdint.h>

#include <classic/common.h>
#include <classic/map.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct ccl_trnode_t {
	void *key;
	void *value;
	struct ccl_trnode_t *parent;
	struct ccl_trnode_t *left;
	struct ccl_trnode_t *right;
	uint32_t priority;
} ccl_trnode;

typedef unsigned (* ccl_prio_cb)(const void *);

typedef struct ccl_trtree_t {
	struct ccl_trnode_t *root;
	ccl_cmp_cb cmp;
	ccl_free_cb kfree;
	ccl_free_cb vfree;
	ccl_prio_cb prio;
	size_t count;
} ccl_trtree;

typedef struct ccl_trtree_iter_t {
	struct ccl_trnode_t *node;
	struct ccl_trtree_t *tree;
} ccl_trtree_iter;

ccl_trtree *ccl_trtree_new(ccl_cmp_cb, ccl_free_cb, ccl_free_cb, ccl_prio_cb);
size_t ccl_trtree_clear(ccl_trtree *tree);
void ccl_trtree_free(ccl_trtree *tree);
bool ccl_trtree_select(ccl_trtree *tree, void *k, void **v);
bool ccl_trtree_insert(ccl_trtree *tree, void *k, void *v, void **);
bool ccl_trtree_unlink(ccl_trtree *tree, void *key, void **k, void **v);
bool ccl_trtree_delete(ccl_trtree *tree, void *k);
bool ccl_trtree_foreach(ccl_trtree *tree, ccl_dforeach_cb cb, void *user);

/* sorted map */
ccl_map *ccl_smap_trtree(ccl_cmp_cb, ccl_free_cb, ccl_free_cb, ccl_prio_cb);

#ifdef  __cplusplus
}
#endif

#endif
