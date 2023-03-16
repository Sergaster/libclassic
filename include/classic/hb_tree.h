/*
   Copyright (C) 2022 Sergey V. Kostyuk

   This file is part of libclassic.
   Author: Sergey V. Kostyuk <kostyuk.sergey79@gmail.com>
   Algo: height-balanced (AVL) tree.

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

#ifndef CCL_HB_TREE_H
#define CCL_HB_TREE_H

#include <stdlib.h>
#include <stdbool.h>

#include <classic/common.h>
#include <classic/map.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct ccl_hbnode_t {
	void *key;
	void *value;
	struct ccl_hbnode_t *parent;
	struct ccl_hbnode_t *left;
	struct ccl_hbnode_t *right;
	unsigned char balance;
} ccl_hbnode;

typedef struct ccl_hbtree_t {
	struct ccl_hbnode_t *root;
	ccl_cmp_cb cmp;
	ccl_free_cb kfree;
	ccl_free_cb vfree;
	size_t count;
} ccl_hbtree;

typedef struct ccl_hbtree_iter_t {
	struct ccl_hbnode_t *node;
	struct ccl_hbtree_t *tree;
} ccl_hbtree_iter;

ccl_hbtree *ccl_hbtree_new(ccl_cmp_cb, ccl_free_cb, ccl_free_cb);
size_t ccl_hbtree_clear(ccl_hbtree *tree);
void ccl_hbtree_free(ccl_hbtree *tree);
bool ccl_hbtree_select(ccl_hbtree *tree, void *k, void **v);
bool ccl_hbtree_insert(ccl_hbtree *tree, void *k, void *v, void **);
bool ccl_hbtree_unlink(ccl_hbtree *tree, void *key, void **k, void **v);
bool ccl_hbtree_delete(ccl_hbtree *tree, void *k);
bool ccl_hbtree_foreach(ccl_hbtree *tree, ccl_dforeach_cb cb, void *user);

/* sorted map */
ccl_map *ccl_smap_hbtree(ccl_cmp_cb, ccl_free_cb, ccl_free_cb);

#ifdef  __cplusplus
}
#endif

#endif
