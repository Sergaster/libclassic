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

#include <string.h>
#include <assert.h>

#include <classic/tr_tree.h>

static ccl_trnode *ccl_trnode_alloc(void* k, void *v)
{
	ccl_trnode *node;

	node = malloc(sizeof(*node));
	if (node == NULL)
		return NULL;
	node->key = k;
	node->value = v;
	node->parent = NULL;
	node->left = NULL;
	node->right = NULL;
	return node;
}

static void ccl_trnode_dealloc(ccl_trnode *node, void **k, void **v)
{
	*k = node->key;
	*v = node->value;
	free(node);
	return;
}

ccl_trtree *ccl_trtree_new(ccl_cmp_cb cmp_cb, ccl_free_cb kfree_cb, ccl_free_cb vfree_cb, ccl_prio_cb prio_cb)
{
	ccl_trtree *tree;

	if (cmp_cb == NULL || prio_cb == NULL)
		return NULL;
	tree = malloc(sizeof(*tree));
	if (tree == NULL)
		return NULL;
	tree->cmp = cmp_cb;
	tree->kfree = kfree_cb;
	tree->vfree = vfree_cb;
	tree->prio = prio_cb;
	tree->count = 0;
	return tree;
}

size_t ccl_trtree_clear(ccl_trtree *tree)
{
	ccl_trnode *node, *p;
	void *k, *v;
	size_t count;

	node = tree->root;
	count = 0;
	while (node) {
		if (node->left) {
			node = node->left;
			continue;
		}

		if (node->right) {
			node = node->right;
			continue;
		}

		p = node->parent;
		ccl_trnode_dealloc(node, &k, &v);
		if (tree->kfree != NULL)
			tree->kfree(k);
		if (tree->vfree != NULL)
			tree->vfree(v);
		if (p == NULL) {
			tree->root = NULL;
		} else {
			if (p->left == node)
				p->left = NULL;
			else
				p->right = NULL;
		}
		tree->count--;
		count++;
		node = p;
	}
	return count;
} 

void ccl_trtree_free(ccl_trtree *tree)
{
	ccl_trtree_clear(tree);
	free(tree);
	return;
}

static ccl_trnode *ccl_trtree_search_node(ccl_trtree *tree, void *k)
{
	ccl_trnode *node;
	int ret;

	if (k == NULL)
		return NULL;
	if (tree->root == NULL)
		return NULL;
	node = tree->root;
	do {
		ret = tree->cmp(k, node->key);
		if (ret < 0)
			node = node->left;
		else if (ret > 0)
			node = node->right;
		else
			break;
	} while (node);
	return node;
}

bool ccl_trtree_select(ccl_trtree *tree, void *k, void **v)
{
	ccl_trnode *node;

	node = ccl_trtree_search_node(tree, k);
	if (node == NULL)
		return false;
	*v = node->value;
	return true;
}

static void ccl_trtree_rot_left(ccl_trtree *tree, ccl_trnode *node)
{
	ccl_trnode *nr, *np;

	nr = node->right;

	node->right = nr->left;
	if (node->right != NULL)
		node->right->parent = node;
	nr->left = node;

	np = node->parent;
	node->parent = nr;
	nr->parent = np;

	if (np == NULL) {
		tree->root = nr;
	} else {
		if (np->left == node)
			np->left = nr;
		else
			np->right = nr;
	}
	return;
}

static void ccl_trtree_rot_right(ccl_trtree *tree, ccl_trnode *node)
{
	ccl_trnode *nl, *np;

	nl = node->left;

	node->left = nl->right;
	if (node->left != NULL)
		node->left->parent = node;
	nl->right = node;

	np = node->parent;
	node->parent = nl;
	nl->parent = np;

	if (np == NULL) {
		tree->root = nl;
	} else {
		if (np->left == node)
			np->left = nl;
		else
			np->right = nl;
	}
	return;
}

bool ccl_trtree_insert(ccl_trtree *tree, void *k, void *v, void **pv)
{
	ccl_trnode *node, *p, *cur;
	int ret;

	*pv = NULL;
	if (k == NULL)
		return false;

	// empty tree
	if (tree->root == NULL) {
		node = ccl_trnode_alloc(k, v);
		if (node == NULL) {
			return false;
		} else {
			node->priority = tree->prio(k);
			tree->root = node;
			goto out;
		}
	}

	// search for the parent
	node = tree->root;
	while (node) {
		ret = tree->cmp(k, node->key);
		if (ret < 0) {
			p = node;
			node = node->left;
		} else if (ret > 0) {
			p = node;
			node = node->right;
		} else {
			*pv = &node->value;
			return false;
		}
	}

	node = ccl_trnode_alloc(k, v);
	if (node == NULL)
		return false;
	node->priority = tree->prio(k);
	node->parent = p;
	if (ret < 0)
		p->left = node;
	else
		p->right = node;

	// fix tree
	cur = node;
	while (p->priority < cur->priority) {
		if (p->left == node)
			ccl_trtree_rot_right(tree, p);
		else
			ccl_trtree_rot_left(tree, p);
		p = cur->parent;
		if (p == NULL)
			break;
	}
out:
	*pv = &node->value;
	tree->count++;
	return true;
}

bool ccl_trtree_unlink(ccl_trtree *tree, void *key, void **k, void **v)
{
	ccl_trnode *node, *rnode;
	ccl_trnode *p, *cnode;          // parent & child of removed node

	node = ccl_trtree_search_node(tree, key);
	if (node == NULL)
		return false;
	*k = node->key;
	*v = node->value;

	// search for removed node && rotation
	if (node->left == NULL || node->right == NULL) {
		rnode = node;
	} else {
		rnode = node;

		while (rnode->left && rnode->right) {
			if (rnode->left->priority > rnode->right->priority)
				ccl_trtree_rot_right(tree, rnode);
			else
				ccl_trtree_rot_left(tree, rnode);
		}
	}

	// link child & parent of removed node
	p = rnode->parent;
	cnode = (rnode->left == NULL ? rnode->right : rnode->left);
	if (cnode)
		cnode->parent = p;
	if (p == NULL) {
		tree->root = cnode;
	} else {
		if (p->left == rnode)
			p->left = cnode;
		else
			p->right = cnode;
	}
	ccl_trnode_dealloc(rnode, k, v);
	tree->count--;
	return true;
}

bool ccl_trtree_delete(ccl_trtree *tree, void *key)
{
	void *k, *v;

	if (!ccl_trtree_unlink(tree, key, &k, &v))
		return false;
	if (tree->kfree != NULL)
		tree->kfree(k);
	if (tree->vfree != NULL)
		tree->vfree(v);
	return true;
}

static ccl_trnode *ccl_trnode_next(ccl_trnode *node)
{
	ccl_trnode *n;

	if (node->right) {
		n = node->right;
		while (n->left)
			n = n->left;
	} else {
		ccl_trnode *p;

		n = node;
		p = n->parent;
		while (p && p->right == n) {
			n = p;
			p = p->parent;
		}
		n = p;
	}
	return n;
}

bool ccl_trtree_foreach(ccl_trtree *tree, ccl_dforeach_cb cb, void *user)
{
	ccl_trnode *node;

	if (tree->root == NULL)
		return true;
	node = tree->root;
	while (node->left)
		node = node->left;
	for (; node != NULL; node = ccl_trnode_next(node)) {
		if (!cb(node->key, node->value, user))
			return false;
	}
	return true;
}

static struct ccl_map_ops map_ops = {
	(ccl_map_free_cb)ccl_trtree_free,
	(ccl_map_clear_cb)ccl_trtree_clear,
	(ccl_map_select_cb)ccl_trtree_select,
	(ccl_map_insert_cb)ccl_trtree_insert,
	(ccl_map_delete_cb)ccl_trtree_delete,
	(ccl_map_foreach_cb)ccl_trtree_foreach,
};

ccl_map *ccl_smap_trtree(ccl_cmp_cb cmp_cb, ccl_free_cb kfree_cb, ccl_free_cb vfree_cb, ccl_prio_cb prio_cb)
{
	ccl_map *map;

	map = malloc(sizeof(*map));
	if (map == NULL)
		return NULL;
	map->obj = ccl_trtree_new(cmp_cb, kfree_cb, vfree_cb, prio_cb);
	if (map->obj == NULL)
		goto err;
	map->ops = &map_ops;
	map->sorted = true;
	return map;
err:
	free(map);
	return NULL;
}
