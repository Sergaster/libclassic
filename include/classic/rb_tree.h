/*
   Copyright (C) 2022 Sergey V. Kostyuk

   This file is part of libclassic.
   Author: Sergey V. Kostyuk <kostyuk.sergey79@gmail.com>
   Algo: red-black tree.

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

#ifndef CCL_RB_TREE_H
#define CCL_RB_TREE_H

#include <stdlib.h>
#include <stdbool.h>

#include <classic/common.h>
#include <classic/map.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct ccl_rbnode_t {
	void *key;
	void *value;
	struct ccl_rbnode_t *parent;
	struct ccl_rbnode_t *left;
	struct ccl_rbnode_t *right;
	bool black;
} ccl_rbnode;

typedef struct ccl_rbtree_t {
	struct ccl_rbnode_t *root;
	ccl_cmp_cb cmp;
	ccl_free_cb kfree;
	ccl_free_cb vfree;
	size_t count;
} ccl_rbtree;

typedef struct ccl_rbtree_iter_t {
	struct ccl_rbnode_t *node;
	struct ccl_rbtree_t *tree;
} ccl_rbtree_iter;

ccl_rbtree *ccl_rbtree_new(ccl_cmp_cb, ccl_free_cb, ccl_free_cb);
size_t ccl_rbtree_clear(ccl_rbtree *tree);
void ccl_rbtree_free(ccl_rbtree *tree);
bool ccl_rbtree_select(ccl_rbtree *tree, void *k, void **v);
bool ccl_rbtree_insert(ccl_rbtree *tree, void *k, void *v, void **);
bool ccl_rbtree_unlink(ccl_rbtree *tree, void *key, void **k, void **v);
bool ccl_rbtree_delete(ccl_rbtree *tree, void *k);
bool ccl_rbtree_foreach(ccl_rbtree *tree, ccl_dforeach_cb cb, void *user);

ccl_map *ccl_map_rbtree(ccl_cmp_cb, ccl_free_cb, ccl_free_cb);

#ifdef  __cplusplus
}
#endif

#endif
