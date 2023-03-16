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

#include <string.h>
#include <assert.h>

#include <classic/wb_tree.h>

static ccl_wbnode *ccl_wbnode_alloc(void* k, void *v, unsigned weight)
{
	ccl_wbnode *node;

	node = malloc(sizeof(*node));
	if (node == NULL)
		return NULL;
	node->key = k;
	node->value = v;
	node->weight = weight;
	node->parent = NULL;
	node->left = NULL;
	node->right = NULL;
	return node;
}

static void ccl_wbnode_dealloc(ccl_wbnode *node, void **k, void **v)
{
	*k = node->key;
	*v = node->value;
	free(node);
	return;
}

ccl_wbtree *ccl_wbtree_new(ccl_cmp_cb cmp_cb, ccl_free_cb kfree_cb, ccl_free_cb vfree_cb)
{
	ccl_wbtree *tree;

	if (cmp_cb == NULL)
		return NULL;
	tree = malloc(sizeof(*tree));
	if (tree == NULL)
		return NULL;
	tree->cmp = cmp_cb;
	tree->kfree = kfree_cb;
	tree->vfree = vfree_cb;
	tree->count = 0;
	return tree;
}

size_t ccl_wbtree_clear(ccl_wbtree *tree)
{
	ccl_wbnode *node, *p;
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
		ccl_wbnode_dealloc(node, &k, &v);
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

void ccl_wbtree_free(ccl_wbtree *tree)
{
	ccl_wbtree_clear(tree);
	free(tree);
	return;
}

static ccl_wbnode *ccl_wbtree_search_node(ccl_wbtree *tree, void *k)
{
	ccl_wbnode *node;
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

bool ccl_wbtree_select(ccl_wbtree *tree, void *k, void **v)
{
	ccl_wbnode *node;

	node = ccl_wbtree_search_node(tree, k);
	if (node == NULL)
		return false;
	*v = node->value;
	return true;
}

static void ccl_wbtree_rot_left(ccl_wbtree *tree, ccl_wbnode *node)
{
	ccl_wbnode *nr, *np;

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

static void ccl_wbtree_rot_right(ccl_wbtree *tree, ccl_wbnode *node)
{
	ccl_wbnode *nl, *np;

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

#define WEIGHT(n)		((n) ? (n)->weight : 1U)

static void ccl_wbtree_ftree_right(ccl_wbtree *tree, ccl_wbnode *node)
{
	ccl_wbnode *nr, *nrl;

	nr = node->right;
	assert(nr != NULL);
	nrl = nr->left;

	if (WEIGHT(nrl) * 1000U < nr->weight * 586U) {
		ccl_wbtree_rot_left(tree, node);
		nr->weight = (node->weight = WEIGHT(node->left) + 
			     WEIGHT(node->right)) + WEIGHT(nr->right);
	} else {
		ccl_wbnode *np, *a, *b;

		assert(nrl != NULL);
		np = node->parent;
		nrl->parent = np;

		if (np == NULL) {
			tree->root = nrl;
		} else {
			if (np->left == node)
				np->left = nrl;
			else
				np->right = nrl;
		}

		a = nrl->left;
		nrl->left = node;
		node->parent = nrl;
		node->right = a;
		if (node->right)
			node->right->parent = node;

		b = nrl->right;
		nrl->right = nr;
		nr->parent = nrl;
		nr->left = b;
		if (nr->left)
			nr->left->parent = nr;

		nrl->weight = (node->weight = WEIGHT(node->left) + 
			      WEIGHT(a)) + (nr->weight = WEIGHT(b) + 
			      WEIGHT(nr->right));
	}
	return;
}

static void ccl_wbtree_ftree_left(ccl_wbtree *tree, ccl_wbnode *node)
{
	ccl_wbnode *nl;
	unsigned weight;

	nl = node->left;
	assert(nl != NULL);
	weight = WEIGHT(nl->left);

	if (weight * 1000U > nl->weight * 414U) {
		ccl_wbtree_rot_right(tree, node);
		node->weight = WEIGHT(node->left) + WEIGHT(node->right);
		nl->weight = weight + node->weight;
	} else {
		ccl_wbnode *nlr, *np, *a, *b;

		nlr = nl->right;
		assert(nlr != NULL);
		np = node->parent;
		nlr->parent = np;

		if (np == NULL) {
			tree->root = nlr;
		} else {
			if (np->left == node)
				np->left = nlr;
			else
				np->right = nlr;
		}

		a = nlr->left;
		nlr->left = nl;
		nl->parent = nlr;
		nl->right = a;
		if (nl->right)
			nl->right->parent = nl;

		b = nlr->right;
		nlr->right = node;
		node->parent = nlr;
		node->left = b;
		if (node->left)
			node->left->parent = node;

		nlr->weight = (node->weight = WEIGHT(b) + 
			      WEIGHT(node->right)) + 
			      (nl->weight = WEIGHT(nl->left) + WEIGHT(a));
	}
	return;
}

static void ccl_wbtree_ftree(ccl_wbtree *tree, ccl_wbnode *node)
{
	unsigned weight = WEIGHT(node->left);

	if (weight * 1000U < node->weight * 293U)
		ccl_wbtree_ftree_right(tree, node);
	else if (weight * 1000U > node->weight * 707U)
		ccl_wbtree_ftree_left(tree, node);
	return;
}

bool ccl_wbtree_insert(ccl_wbtree *tree, void *k, void *v, void **pv)
{
	ccl_wbnode *node, *p, *cur;
	int ret;

	*pv = NULL;
	if (k == NULL)
		return false;

	// empty tree
	if (tree->root == NULL) {
		node = ccl_wbnode_alloc(k, v, 2);
		if (node == NULL) {
			return false;
		} else {
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

	node = ccl_wbnode_alloc(k, v, 2);
	if (node == NULL)
		return false;
	node->parent = p;
	if (ret < 0)
		p->left = node;
	else
		p->right = node;

	// fix tree
	cur = p;
	while (cur != NULL) {
		p = cur->parent;
		cur->weight++;
		ccl_wbtree_ftree(tree, cur);
		cur = p;
	}
out:
	*pv = &node->value;
	tree->count++;
	return true;
}

bool ccl_wbtree_unlink(ccl_wbtree *tree, void *key, void **k, void **v)
{
	ccl_wbnode *node, *rnode;
	ccl_wbnode *p, *g, *cnode;          // parent & child of removed node

	node = ccl_wbtree_search_node(tree, key);
	if (node == NULL)
		return false;
	*k = node->key;
	*v = node->value;

	// search for removed node
	if (node->left == NULL || node->right == NULL) {
		rnode = node;
	} else {
		void *tmp;

		if (node->left->weight > node->right->weight) {
			rnode = node->left;
			while (rnode->right)
				rnode = rnode->right;
		} else {
			rnode = node->right;
			while (rnode->left)
				rnode = rnode->left;
		}
		tmp = node->key;
		node->key = rnode->key;
		rnode->key = tmp;
		tmp = node->value;
		node->value = rnode->value;
		rnode->value = tmp;
	}

	assert(rnode->left == NULL || rnode->right == NULL);

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

	// fix tree
	while (p != NULL) {
		g = p->parent;
		p->weight--;
		ccl_wbtree_ftree(tree, p);
		p = g;
	}
	ccl_wbnode_dealloc(rnode, k, v);
	tree->count--;
	return true;
}

bool ccl_wbtree_delete(ccl_wbtree *tree, void *key)
{
	void *k, *v;

	if (!ccl_wbtree_unlink(tree, key, &k, &v))
		return false;
	if (tree->kfree != NULL)
		tree->kfree(k);
	if (tree->vfree != NULL)
		tree->vfree(v);
	return true;
}

static ccl_wbnode *ccl_wbnode_next(ccl_wbnode *node)
{
	ccl_wbnode *n;

	if (node->right) {
		n = node->right;
		while (n->left)
			n = n->left;
	} else {
		ccl_wbnode *p;

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

bool ccl_wbtree_foreach(ccl_wbtree *tree, ccl_dforeach_cb cb, void *user)
{
	ccl_wbnode *node;

	if (tree->root == NULL)
		return true;
	node = tree->root;
	while (node->left)
		node = node->left;
	for (; node != NULL; node = ccl_wbnode_next(node)) {
		if (!cb(node->key, node->value, user))
			return false;
	}
	return true;
}

static struct ccl_map_ops map_ops = {
	(ccl_map_free_cb)ccl_wbtree_free,
	(ccl_map_clear_cb)ccl_wbtree_clear,
	(ccl_map_select_cb)ccl_wbtree_select,
	(ccl_map_insert_cb)ccl_wbtree_insert,
	(ccl_map_delete_cb)ccl_wbtree_delete,
	(ccl_map_foreach_cb)ccl_wbtree_foreach,
};

ccl_map *ccl_smap_wbtree(ccl_cmp_cb cmp_cb, ccl_free_cb kfree_cb, ccl_free_cb vfree_cb)
{
	ccl_map *map;

	map = malloc(sizeof(*map));
	if (map == NULL)
		return NULL;
	map->obj = ccl_wbtree_new(cmp_cb, kfree_cb, vfree_cb);
	if (map->obj == NULL)
		goto err;
	map->ops = &map_ops;
	map->sorted = true;
	return map;
err:
	free(map);
	return NULL;
}
