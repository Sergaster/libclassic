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

#include <string.h>

#include <classic/rb_tree.h>

static ccl_rbnode *ccl_rbnode_alloc(void *k, void *v, bool black)
{
	ccl_rbnode *node;

	node = malloc(sizeof(*node));
	if (node == NULL)
		return NULL;
	node->key = k;
	node->value = v;
	node->black = black;
	node->parent = NULL;
	node->left = NULL;
	node->right = NULL;
	return node;
}

static void ccl_rbnode_dealloc(ccl_rbnode *node, void **k, void **v)
{
	*k = node->key;
	*v = node->value;
	free(node);
	return;
}

ccl_rbtree *ccl_rbtree_new(ccl_cmp_cb cmp_cb, ccl_free_cb kfree_cb, ccl_free_cb vfree_cb)
{
	ccl_rbtree *tree;

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

size_t ccl_rbtree_clear(ccl_rbtree *tree)
{
	ccl_rbnode *node, *p;
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
		ccl_rbnode_dealloc(node, &k, &v);
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

void ccl_rbtree_free(ccl_rbtree *tree)
{
	ccl_rbtree_clear(tree);
	free(tree);
	return;
}

static ccl_rbnode *ccl_rbnode_next(ccl_rbnode *node)
{
	ccl_rbnode *n;

	if (node->right) {
		n = node->right;
		while (n->left)
			n = n->left;
	} else {
        	ccl_rbnode *p;

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

static ccl_rbnode *ccl_rbnode_prev(ccl_rbnode *node)
{
	if (node->left) {
		for (node = node->left; node->right; node = node->right)
			;
	} else {
		ccl_rbnode *p  = node->parent;
		while (p && p->left == node) {
			node = p;
			p = p->parent;
		}
		node = p;
	}
	return node;
}

static ccl_rbnode *ccl_rbtree_search_node(ccl_rbtree *tree, void *k)
{
	ccl_rbnode *node;
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

bool ccl_rbtree_select(ccl_rbtree *tree, void *k, void **v)
{
	ccl_rbnode *node;

	node = ccl_rbtree_search_node(tree, k);
	if (node == NULL)
		return false;
	*v = node->value;
	return true;
}

static void ccl_rbtree_rot_left(ccl_rbtree *tree, ccl_rbnode *node)
{
	ccl_rbnode *nr, *np;

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

static void ccl_rbtree_rot_right(ccl_rbtree *tree, ccl_rbnode *node)
{
	ccl_rbnode *nl, *np;

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

static ccl_rbnode *ccl_rbtree_insert_fleft(ccl_rbtree *tree, ccl_rbnode *node, ccl_rbnode *p, ccl_rbnode *g)
{
	ccl_rbnode *u;

	u = g->right;
	if (u != NULL && !u->black) {
		u->black = true;
		p->black = true;
		g->black = false;
		node = g;
	} else {
		if (node == p->right) {
			ccl_rbnode *n;
			node = p;
			ccl_rbtree_rot_left(tree, node);
			n = node->parent;
			n->black = true;
			n = n->parent;
			n->black = false;
			ccl_rbtree_rot_right(tree, n);
			p = node->parent;
		} else {
			ccl_rbnode *n;
			n = node->parent;
			n->black = true;
			n = n->parent;
			n->black = false;
			ccl_rbtree_rot_right(tree, n);
		}
	}
	return node;
}

static ccl_rbnode *ccl_rbtree_insert_fright(ccl_rbtree *tree, ccl_rbnode *node, ccl_rbnode *p, ccl_rbnode *g)
{
	ccl_rbnode *u;

	u = g->left;
	if (u != NULL && !u->black) {
		u->black = true;
		p->black = true;
		g->black = false;
		node = g;
	} else {
		if (node == p->left) {
			ccl_rbnode *n;
			node = p;
			ccl_rbtree_rot_right(tree, node);
			n = node->parent;
			n->black = true;
			n = n->parent;
			n->black = false;
			ccl_rbtree_rot_left(tree, n);
		} else {
			ccl_rbnode *n;
			n = node->parent;
			n->black = true;
			n = n->parent;
			n->black = false;
			ccl_rbtree_rot_left(tree, n);
		}
	}
	return node;
}

static void ccl_rbtree_insert_ftree(ccl_rbtree *tree, ccl_rbnode *node)
{
	ccl_rbnode *p, *g;		// parent, grandparent

	do {
		p = node->parent;
		g = p->parent;
		if (g == NULL)
			break;
		if (p == g->left)
			node = ccl_rbtree_insert_fleft(tree, node, p, g);
		else
			node = ccl_rbtree_insert_fright(tree, node, p, g);
	} while (node != tree->root && !node->parent->black);

	tree->root->black = true;
	return;
}

bool ccl_rbtree_insert(ccl_rbtree *tree, void *k, void *v, void **pv)
{
	ccl_rbnode *node, *p;
	int ret;

	*pv = NULL;
	if (k == NULL)
		return false;

	// empty tree
	if (tree->root == NULL) {
		node = ccl_rbnode_alloc(k, v, true);
		if (node == NULL) {
			return false;
		} else {
			node->black = true;
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

	node = ccl_rbnode_alloc(k, v, false);
	if (node == NULL)
		return false;
	node->parent = p;
	if (ret < 0)
		p->left = node;
	else
		p->right = node;

	if (!p->black)		// fix tree
		ccl_rbtree_insert_ftree(tree, node);
out:
	*pv = &node->value;
	tree->count++;
	return true;
}

static void ccl_rbtree_unlink_ftree(ccl_rbtree *tree, ccl_rbnode *n, ccl_rbnode *p, bool dir)
{
	ccl_rbnode *w;

	while (n != tree->root && (n == NULL || n->black)) {
		if (dir) {
			w = p->right;
			if (!w->black) {
				w->black = true;
				p->black = false;
				ccl_rbtree_rot_left(tree, p);
				w = p->right;
			}

			if ((w->left == NULL || w->left->black) &&
			    (w->right == NULL || w->right->black)) {
				w->black = false;
				n = p;
				p = p->parent;
				if (p != NULL && p->left == n)
					dir = true;
				else
					dir = false;
			} else {
				if (w->right == NULL || w->right->black) {
					w->left->black = true;
					w->black = false;
					ccl_rbtree_rot_right(tree, w);
					w = p->right;
				}

				if (p->black)
					w->black = true;
				else
					w->black = false;
				if (w->right != NULL)
					w->right->black = true;
				p->black = true;
				ccl_rbtree_rot_left(tree, p);
				break;
			}
		} else {
			w = p->left;
			if (!w->black) {
				w->black = true;
				p->black = false;
				ccl_rbtree_rot_right(tree, p);
				w = p->left;
			}

			if ((w->left == NULL || w->left->black) &&
			    (w->right == NULL || w->right->black)) {
				w->black = false;
				n = p;
				p = p->parent;
				if (p != NULL && p->left == n)
					dir = true;
				else
					dir = false;
			} else {
				if (w->left == NULL || w->left->black) {
					w->right->black = true;
					w->black = false;
					ccl_rbtree_rot_left(tree, w);
					w = p->left;
				}

				if (p->black)
					w->black = true;
				else
					w->black = false;
				if (w->left != NULL)
					w->left->black = true;
				p->black = true;
				ccl_rbtree_rot_right(tree, p);
				break;
			}
		}
	}
	if (n)
		n->black = true;
	return;
}

bool ccl_rbtree_unlink(ccl_rbtree *tree, void *key, void **k, void **v)
{
	ccl_rbnode *node, *rnode;
	ccl_rbnode *p, *cnode;		// parent & child of removed node
	bool dir;

	node = ccl_rbtree_search_node(tree, key);
	if (node == NULL)
		return false;
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
	if (cnode != NULL)
		cnode->parent = p;
	if (p == NULL) {
		tree->root = cnode;
		dir = false;
	} else {
		if (p->left == rnode) {
			p->left = cnode;
			dir = true;
		} else {
			p->right = cnode;
			dir = false;
		}
	}

	if (rnode->black && tree->root != NULL)
		ccl_rbtree_unlink_ftree(tree, cnode, p, dir);
	ccl_rbnode_dealloc(rnode, k, v);
	tree->count--;
	return true;
}
 
bool ccl_rbtree_delete(ccl_rbtree *tree, void *key)
{
	void *k, *v;

	if (!ccl_rbtree_unlink(tree, key, &k, &v))
		return false;
	if (tree->kfree != NULL)
		tree->kfree(k);
	if (v != NULL && tree->vfree != NULL)
		tree->vfree(v);
	return true;
}

bool ccl_rbtree_foreach(ccl_rbtree *tree, ccl_dforeach_cb cb, void *user) 
{
	ccl_rbnode *node;

	if (tree->root == NULL)
		return true;
	node = tree->root;
	while (node->left)
		node = node->left;
	for (; node != NULL; node = ccl_rbnode_next(node)) {
		if (!cb(node->key, node->value, user))
			return false;
	}
	return true;
}

static struct ccl_map_ops map_ops = {
	(ccl_map_free_cb)ccl_rbtree_free,
	(ccl_map_clear_cb)ccl_rbtree_clear,
	(ccl_map_select_cb)ccl_rbtree_select,
	(ccl_map_insert_cb)ccl_rbtree_insert,
	(ccl_map_delete_cb)ccl_rbtree_delete,
	(ccl_map_foreach_cb)ccl_rbtree_foreach,
};

ccl_map *ccl_map_rbtree(ccl_cmp_cb cmp_cb, ccl_free_cb kfree_cb, ccl_free_cb vfree_cb)
{
	ccl_map *map;

	map = malloc(sizeof(*map));
	if (map == NULL)
		return NULL;
	map->obj = ccl_rbtree_new(cmp_cb, kfree_cb, vfree_cb);
	if (map->obj == NULL)
		goto err;
	map->ops = &map_ops;
	map->sorted = true;
	return map;
err:
	free(map);
	return NULL;
}
