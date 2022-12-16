/*
   Copyright (C) 2022 Sergey V. Kostyuk

   This file is part of libclassic.
   Author: Sergey V. Kostyuk <kostyuk.sergey79@gmail.com>
   Algo: weight-balanced tree.
   Ref: [Gonnet 1984], [Nievergelt and Reingold 1973].

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

#ifndef CCL_WB_TREE_H
#define CCL_WB_TREE_H

#include <stdlib.h>
#include <stdint.h>

#include <classic/common.h>
#include <classic/map.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct ccl_wbnode_t {
	void *key;
	void *value;
	struct ccl_wbnode_t *parent;
	struct ccl_wbnode_t *left;
	struct ccl_wbnode_t *right;
	uint32_t weight;
} ccl_wbnode;

typedef struct ccl_wbtree_t {
	struct ccl_wbnode_t *root;
	ccl_cmp_cb cmp;
	ccl_free_cb kfree;
	ccl_free_cb vfree;
	size_t count;
} ccl_wbtree;

typedef struct ccl_wbtree_iter_t {
	struct ccl_wbnode_t *node;
	struct ccl_wbtree_t *tree;
} ccl_wbtree_iter;

ccl_wbtree *ccl_wbtree_new(ccl_cmp_cb, ccl_free_cb, ccl_free_cb);
size_t ccl_wbtree_clear(ccl_wbtree *tree);
void ccl_wbtree_free(ccl_wbtree *tree);
bool ccl_wbtree_select(ccl_wbtree *tree, void *k, void **v);
bool ccl_wbtree_insert(ccl_wbtree *tree, void *k, void *v, void **);
bool ccl_wbtree_unlink(ccl_wbtree *tree, void *key, void **k, void **v);
bool ccl_wbtree_delete(ccl_wbtree *tree, void *k);
bool ccl_wbtree_foreach(ccl_wbtree *tree, ccl_dforeach_cb cb, void *user);

ccl_map *ccl_map_wbtree(ccl_cmp_cb, ccl_free_cb, ccl_free_cb);

#ifdef  __cplusplus
}
#endif

#endif
