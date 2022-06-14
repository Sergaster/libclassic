/*
   Copyright (C) 2022 Sergey V. Kostyuk

   This file is part of libclassic.
   Author: Sergey V. Kostyuk <kostyuk.sergey79@gmail.com>
   Algo: internal path reduction tree.
   Ref: [Gonnet 1983], [Gonnet 1984].

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

#ifndef CCL_PR_TREE_H
#define CCL_PR_TREE_H

#include <stdlib.h>

#include <classic/common.h>
#include <classic/map.h>

typedef struct ccl_prnode_t {
	void *key;
	void *value;
	struct ccl_prnode_t *parent;
	struct ccl_prnode_t *left;
	struct ccl_prnode_t *right;
	unsigned weight;
} ccl_prnode;

typedef struct ccl_prtree_t {
	struct ccl_prnode_t *root;
	ccl_cmp_cb cmp;
	ccl_free_cb kfree;
	ccl_free_cb vfree;
	size_t count;
} ccl_prtree;

typedef struct ccl_prtree_iter_t {
	struct ccl_prnode_t *node;
	struct ccl_prtree_t *tree;
} ccl_prtree_iter;

ccl_prtree *ccl_prtree_new(ccl_cmp_cb, ccl_free_cb, ccl_free_cb);
size_t ccl_prtree_clear(ccl_prtree *tree);
void ccl_prtree_free(ccl_prtree *tree);
int ccl_prtree_select(ccl_prtree *tree, void *k, void **v);
int ccl_prtree_insert(ccl_prtree *tree, void *k, void *v, void **);
int ccl_prtree_unlink(ccl_prtree *tree, void *key, void **k, void **v);
int ccl_prtree_delete(ccl_prtree *tree, void *k);
int ccl_prtree_foreach(ccl_prtree *tree, ccl_foreach_cb cb, void *user);

ccl_map *ccl_map_prtree(ccl_cmp_cb, ccl_free_cb, ccl_free_cb);

#endif
