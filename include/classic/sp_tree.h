/*
   Copyright (C) 2022 Sergey V. Kostyuk

   This file is part of libclassic.
   Author: Sergey V. Kostyuk <kostyuk.sergey79@gmail.com>
   Algo: splay tree.
   Ref: [Sleator and Tarjan, 1985], [Tarjan 1985], [Tarjan 1983].

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

#ifndef CCL_SP_TREE_H
#define CCL_SP_TREE_H

#include <stdlib.h>

#include <classic/common.h>
#include <classic/map.h>

typedef struct ccl_spnode_t {
	void *key;
	void *value;
	struct ccl_spnode_t *parent;
	struct ccl_spnode_t *left;
	struct ccl_spnode_t *right;
} ccl_spnode;

typedef struct ccl_sptree_t {
	struct ccl_spnode_t *root;
	ccl_cmp_cb cmp;
	ccl_free_cb kfree;
	ccl_free_cb vfree;
	size_t count;
} ccl_sptree;

typedef struct ccl_sptree_iter_t {
	struct ccl_spnode_t *node;
	struct ccl_sptree_t *tree;
} ccl_sptree_iter;

ccl_sptree *ccl_sptree_new(ccl_cmp_cb, ccl_free_cb, ccl_free_cb);
size_t ccl_sptree_clear(ccl_sptree *tree);
void ccl_sptree_free(ccl_sptree *tree);
int ccl_sptree_select(ccl_sptree *tree, void *k, void **v);
int ccl_sptree_insert(ccl_sptree *tree, void *k, void *v, void **);
int ccl_sptree_unlink(ccl_sptree *tree, void *key, void **k, void **v);
int ccl_sptree_delete(ccl_sptree *tree, void *k);
int ccl_sptree_foreach(ccl_sptree *tree, ccl_dforeach_cb cb, void *user);

ccl_map *ccl_map_sptree(ccl_cmp_cb, ccl_free_cb, ccl_free_cb);

#endif
