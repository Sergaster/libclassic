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

#include <string.h>

#include <classic/sp_tree.h>

static ccl_spnode *ccl_spnode_alloc(void* k, void *v)
{
	ccl_spnode *node;

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

static void ccl_spnode_dealloc(ccl_spnode *node, void **k, void **v)
{
	*k = node->key;
	*v = node->value;
	free(node);
	return;
}

ccl_sptree *ccl_sptree_new(ccl_cmp_cb cmp_cb, ccl_free_cb kfree_cb, ccl_free_cb vfree_cb)
{
	ccl_sptree *tree;

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

size_t ccl_sptree_clear(ccl_sptree *tree)
{
	ccl_spnode *node, *p;
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
		ccl_spnode_dealloc(node, &k, &v);
		if (tree->kfree != NULL)
			tree->kfree(k);
		if (v != NULL && tree->vfree != NULL)
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

void ccl_sptree_free(ccl_sptree *tree)
{
	ccl_sptree_clear(tree);
	free(tree);
	return;
}

static void ccl_sptree_splay(ccl_sptree *tree, ccl_spnode *node)
{
	ccl_spnode *p, *g, *gg;

	for (;;) {
		p = node->parent;
		if (p == NULL)
			break;
		g = p->parent;
		if (g == NULL) {
			if (p->left == node) {
				p->left = node->right;
				if (p->left != NULL)
					p->left->parent = p;
				node->right = p;
			} else {
				p->right = node->left;
				if (p->right != NULL)
					p->right->parent = p;
				node->left = p;
			}
			p->parent = node;
			tree->root = node;
			node->parent = NULL;
			break;
		}

		gg = g->parent;
		if (p->left == node) {
			if (g->left == p) {
				ccl_spnode *pr, *nr;

				pr = p->right;
				p->right = g;
				g->parent = p;
				g->left = pr;
				if (g->left)
					g->left->parent = g;

				nr = node->right;
				node->right = p;
				p->parent = node;
				p->left = nr;
				if (p->left)
					p->left->parent = p;
			} else {
				ccl_spnode *nr, *nl;

				nr = node->right;
				node->right = p;
				p->parent = node;
				p->left = nr;
				if (p->left)
					p->left->parent = p;

				nl = node->left;
				node->left = g;
				g->parent = node;
				g->right = nl;
				if (g->right)
					g->right->parent = g;
			}
		} else {
			if (g->right == p) {
				ccl_spnode *pl, *nl;

				pl = p->left;
				p->left = g;
				g->parent = p;
				g->right = pl;
				if (g->right)
					g->right->parent = g;

				nl = node->left;
				node->left = p;
				p->parent = node;
				p->right = nl;
				if (p->right)
					p->right->parent = p;
			} else {
				ccl_spnode *nl, *nr;

				nl = node->left;
				node->left = p;
				p->parent = node;
				p->right = nl;
				if (p->right)
					p->right->parent = p;

				nr = node->right;
				node->right = g;
				g->parent = node;
				g->left = nr;
				if (g->left)
					g->left->parent = g;
			}
		}
		node->parent = gg;
		if (gg) {
			if (gg->left == g)
				gg->left = node;
			else
				gg->right = node;
		} else {
			tree->root = node;
			break;
		}
	}
	return;
}

static ccl_spnode *ccl_sptree_search_node(ccl_sptree *tree, void *k)
{
	ccl_spnode *node;
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

int ccl_sptree_select(ccl_sptree *tree, void *k, void **v)
{
	ccl_spnode *node, *np;
	int ret;

	if (k == NULL)
		return -1;
	if (tree->root == NULL)
		return -1;
	node = tree->root;
	do {
		np = node;
		ret = tree->cmp(k, node->key);
		if (ret < 0) {
			node = node->left;
		} else if (ret > 0) {
			node = node->right;
		} else {
			ccl_sptree_splay(tree, node);
			*v = node->value;
			return 0;;
		}
	} while (node);

	ccl_sptree_splay(tree, np);
	return -1;
}

int ccl_sptree_insert(ccl_sptree *tree, void *k, void *v, void **pv)
{
	ccl_spnode *node, *p;
	int ret;

	*pv = NULL;
	if (k == NULL)
		return -1;

	// empty tree
	if (tree->root == NULL) {
		node = ccl_spnode_alloc(k, v);
		if (node == NULL) {
			return -1;
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
			return -1;
		}
	}

	node = ccl_spnode_alloc(k, v);
	if (node == NULL)
		return -1;
	node->parent = p;
	if (ret < 0)
		p->left = node;
	else
		p->right = node;

	// fix tree
	ccl_sptree_splay(tree, node);
out:
	*pv = &node->value;
	tree->count++;
	return 0;
}

int ccl_sptree_unlink(ccl_sptree *tree, void *key, void **k, void **v)
{
	ccl_spnode *node, *rnode;
	ccl_spnode *p, *cnode;          // parent & child of removed node

	node = ccl_sptree_search_node(tree, key);
	if (node == NULL)
		return -1;
	*k = node->key;
	*v = node->value;

	// search for removed node
	if (node->left == NULL || node->right == NULL) {
		rnode = node;
	} else {
		void *tmp;

		rnode = node->right;
		while (rnode->left)
			rnode = rnode->left;

		tmp = node->key;
		node->key = rnode->key;
		rnode->key = tmp;
		tmp = node->value;
		node->value = rnode->value;
		rnode->value = tmp;
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

	// fix tree
	if (p != NULL)
		ccl_sptree_splay(tree, p);
	ccl_spnode_dealloc(rnode, k, v);
	tree->count--;
	return 0;
}

int ccl_sptree_delete(ccl_sptree *tree, void *key)
{
	void *k, *v;

	if (ccl_sptree_unlink(tree, key, &k, &v))
		return -1;
	if (tree->kfree != NULL)
		tree->kfree(k);
	if (v != NULL && tree->vfree != NULL)
		tree->vfree(v);
	return 0;
}

static ccl_spnode *ccl_spnode_next(ccl_spnode *node)
{
	ccl_spnode *n;

	if (node->right) {
		n = node->right;
		while (n->left)
			n = n->left;
	} else {
		ccl_spnode *p;

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

int ccl_sptree_foreach(ccl_sptree *tree, ccl_foreach_cb cb, void *user)
{
	ccl_spnode *node;

	if (tree->root == NULL)
		return 0;
	node = tree->root;
	while (node->left)
		node = node->left;
	for (; node != NULL; node = ccl_spnode_next(node)) {
		if (cb(node->key, node->value, user))
			return -1;
	}
	return 0;
}

static struct ccl_map_ops map_ops = {
	(ccl_map_free_cb)ccl_sptree_free,
	(ccl_map_clear_cb)ccl_sptree_clear,
	(ccl_map_select_cb)ccl_sptree_select,
	(ccl_map_insert_cb)ccl_sptree_insert,
	(ccl_map_delete_cb)ccl_sptree_delete,
	(ccl_map_foreach_cb)ccl_sptree_foreach,
};

ccl_map *ccl_map_sptree(ccl_cmp_cb cmp_cb, ccl_free_cb kfree_cb, ccl_free_cb vfree_cb)
{
	ccl_map *map;

	map = malloc(sizeof(*map));
	if (map == NULL)
		return NULL;
	map->obj = ccl_sptree_new(cmp_cb, kfree_cb, vfree_cb);
	if (map->obj == NULL)
		goto err;
	map->ops = &map_ops;
	map->sorted = true;
	return map;
err:
	free(map);
	return NULL;
}
