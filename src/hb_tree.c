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

#include <string.h>
#include <assert.h>

#include <classic/hb_tree.h>

#define BAL_POS			0x1
#define BAL_NEG			0x2

static ccl_hbnode *ccl_hbnode_alloc(void* k, void *v)
{
	ccl_hbnode *node;

	node = malloc(sizeof(*node));
	if (node == NULL)
		return NULL;
	node->key = k;
	node->value = v;
	node->parent = NULL;
	node->left = NULL;
	node->right = NULL;
	node->balance = 0;
	return node;
}

static void ccl_hbnode_dealloc(ccl_hbnode *node, void **k, void **v)
{
	*k = node->key;
	*v = node->value;
	free(node);
	return;
}

ccl_hbtree *ccl_hbtree_new(ccl_cmp_cb cmp_cb, ccl_free_cb kfree_cb, ccl_free_cb vfree_cb)
{
	ccl_hbtree *tree;

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

size_t ccl_hbtree_clear(ccl_hbtree *tree)
{
	ccl_hbnode *node, *p;
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
		ccl_hbnode_dealloc(node, &k, &v);
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

void ccl_hbtree_free(ccl_hbtree *tree)
{
	ccl_hbtree_clear(tree);
	free(tree);
	return;
}

static ccl_hbnode *ccl_hbtree_search_node(ccl_hbtree *tree, void *k)
{
	ccl_hbnode *node;
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

bool ccl_hbtree_select(ccl_hbtree *tree, void *k, void **v)
{
	ccl_hbnode *node;

	node = ccl_hbtree_search_node(tree, k);
	if (node == NULL)
		return false;
	*v = node->value;
	return true;
}

static int ccl_hbtree_rot_left(ccl_hbtree *tree, ccl_hbnode *node)
{
	ccl_hbnode *np, *nr, *nrl;
	unsigned char nr_bal;

	assert(node->balance & BAL_POS);
	np = node->parent;
	nr = node->right;
	nr_bal = nr->balance;

	nrl = nr->left;

	// update node
	node->parent = nr;
	node->balance = (nr_bal == 0 ? BAL_POS : 0x0);
	node->right = nrl;
	
	// update nrl
	if (nrl != NULL)
		nrl->parent = node;

	// update right child, etc
	nr->parent = np;
	nr->balance = (nr_bal == 0 ? BAL_NEG : 0x0);
	if (np == NULL) {
		tree->root = nr;
	} else {
		if (np->left == node)
			np->left = nr;
		else
			np->right = nr;
	}
	nr->left = node;
	return (nr_bal ? 0 : 1);
}

static int ccl_hbtree_rot_right(ccl_hbtree *tree, ccl_hbnode *node)
{
	ccl_hbnode *np, *nl, *nlr;
	unsigned char nl_bal;

	assert(node->balance & BAL_NEG);
	np = node->parent;
	nl = node->left;
	nl_bal = nl->balance;

	nlr = nl->right;

	// update node
	node->parent = nl;
	node->balance = (nl_bal == 0 ? BAL_NEG : 0x0);
	node->left = nlr;

	// update nlr
	if (nlr != NULL)
		nlr->parent = node;

	// update left child, etc
	nl->parent = np;
	nl->balance = (nl_bal == 0 ? BAL_POS : 0x0);
	if (np == NULL) {
		tree->root = nl;
	} else {
		if (np->left == node)
			np->left = nl;
		else
			np->right = nl;
	}
	nl->right = node;
	return (nl_bal ? 0 : 1);
}

static void ccl_hbtree_rot_rl(ccl_hbtree *tree, ccl_hbnode *node)
{
	ccl_hbnode *np, *nr, *nrl;
	ccl_hbnode *nrll, *nrlr;
	unsigned char nrl_bal;

	assert(node->balance & BAL_POS);
	nr = node->right;
	assert(nr->balance & BAL_NEG);
	np = node->parent;
	nrl = nr->left;
	nrl_bal = nrl->balance;

	nrll = nrl->left;
	nrlr = nrl->right;

	// update nrl
	nrl->parent = np;
	nrl->balance = 0x0;
	if (np == NULL) {
		tree->root = nrl;
	} else {
		if (np->left == node)
			np->left = nrl;
		else
			np->right = nrl;
	}
	nrl->left = node;
	nrl->right = nr;

	// update node
	node->parent = nrl;
	node->balance = (nrl_bal == 1 ? BAL_NEG : 0x0);
	node->right = nrll;

	// update nrll
	if (nrll != NULL)
		nrll->parent = node;

	// update nr
	nr->parent = nrl;
	nr->balance = (nrl_bal == 2 ? BAL_POS : 0x0);
	nr->left = nrlr;

	// update nrlr
	if (nrlr != NULL)
		nrlr->parent = nr;
	return;
}

static void ccl_hbtree_rot_lr(ccl_hbtree *tree, ccl_hbnode *node)
{
	ccl_hbnode *np, *nl, *nlr;
	ccl_hbnode *nlrl, *nlrr;
	unsigned char nlr_bal;

	assert(node->balance & BAL_NEG);
	nl = node->left;
	assert(nl->balance & BAL_POS);
	np = node->parent;
	nlr = nl->right;
	nlr_bal = nlr->balance;

	nlrl = nlr->left;
	nlrr = nlr->right;

	// update nlr
	nlr->parent = np;
	nlr->balance = 0x0;
	if (np == NULL) {
		tree->root = nlr;
	} else {
		if (np->left == node)
			np->left = nlr;
		else
			np->right = nlr;
	}

	nlr->left = nl;
	nlr->right = node;

	// update node
	node->parent = nlr;
	node->balance = (nlr_bal == 2 ? BAL_POS : 0x0);
	node->left = nlrr;

	// update nlrr
	if (nlrr != NULL)
		nlrr->parent = node;
	// update nl
	nl->parent = nlr;
	nl->balance = (nlr_bal == 1 ? BAL_NEG : 0x0);
	nl->right = nlrl;

	// update nlrl
	if (nlrl != NULL)
		nlrl->parent = nl;
	return;
}

static void ccl_hbtree_insert_ftree(ccl_hbtree *tree, ccl_hbnode *node, ccl_hbnode *n)
{
	ccl_hbnode *p;

	p = node->parent;
	while (p != n) {
		assert(p->balance == 0);
		if (p->left == node)
			p->balance |= BAL_NEG;
		else
			p->balance |= BAL_POS;
		node = p;
		p = p->parent; 
	}

	if (n == NULL)
		return;
	assert(n->balance);

	if (n->left == node) {
		if (n->balance & BAL_NEG) {
			if (n->left->balance & BAL_POS)
				ccl_hbtree_rot_lr(tree, n);
			else
				assert(!ccl_hbtree_rot_right(tree, n));
		} else {
			assert(n->balance & BAL_POS);
			n->balance = 0;
		}
	} else {
		assert(n->right == node);
		if (n->balance & BAL_POS) {
			if (n->right->balance & BAL_NEG)
				ccl_hbtree_rot_rl(tree, n);
			else
				assert(!ccl_hbtree_rot_left(tree, n));
		} else {
			assert(n->balance & BAL_NEG);
			n->balance = 0;
		}
	}
	return;
}

bool ccl_hbtree_insert(ccl_hbtree *tree, void *k, void *v, void **pv)
{
	ccl_hbnode *node, *p, *n;
	int cmp;

	*pv = NULL;
	if (k == NULL)
		return false;
	// empty tree
	if (tree->root == NULL) {
		node = ccl_hbnode_alloc(k, v);
		if (node == NULL) {
			return false;
		} else {
			node->balance = 0x0;
			tree->root = node;
			goto out;
		}
	}

	// search for the parent
	node = tree->root;
	n = NULL;
	cmp = 0;
	while (node) {
		cmp = tree->cmp(k, node->key);
		if (cmp < 0) {
			p = node;
			node = node->left;
		} else if (cmp > 0) {
			p = node;
			node = node->right;
		} else {
			*pv = &node->value;
			return false;
		}

		if (p->balance)
			n = p;
	}

	node = ccl_hbnode_alloc(k, v);
	if (node == NULL)
		return false;
	node->parent = p;

	if (cmp < 0)
		p->left = node;
	else
		p->right = node;

	ccl_hbtree_insert_ftree(tree, node, n);
out:
	*pv = &node->value;
	tree->count++;
	return true;
}

static void ccl_hbtree_unlink_ftree(ccl_hbtree *tree, ccl_hbnode *node, ccl_hbnode *p, bool dir)
{
	for (;;) {
		if (dir) {
			assert(p->left == node);
			if (p->balance & BAL_POS) {
				if (p->right->balance & BAL_NEG) {
					ccl_hbtree_rot_rl(tree, p);
				} else {
					if (ccl_hbtree_rot_left(tree, p))
						break;
				}
				node = p->parent;
			} else if (p->balance & BAL_NEG) {
				p->balance = 0x0;
				node = p;
			} else {
				assert(p->balance == 0);
				p->balance |= BAL_POS;
				break;
			}
		} else {
			assert(p->right == node);
			if (p->balance & BAL_NEG) {
				if (p->left->balance & BAL_POS) {
					ccl_hbtree_rot_lr(tree, p);
				} else {
					if (ccl_hbtree_rot_right(tree, p))
						break;
				}
				node = p->parent;
			} else if (p->balance & BAL_POS) {
				p->balance = 0x0;
				node = p;
			} else {
				assert(p->balance == 0);
				p->balance |= BAL_NEG;
				break;
			}
		}

		p = node->parent;
		if (p == NULL)
			break;
		if (p->left == node) {
			dir = true;
		} else {
			assert(p->right == node);
			dir = false;
		}
	}
	return;
}

bool ccl_hbtree_unlink(ccl_hbtree *tree, void *key, void **k, void **v)
{
	ccl_hbnode *node, *rnode;
	ccl_hbnode *p, *cnode;          // parent & child of removed node
	bool dir;

	node = ccl_hbtree_search_node(tree, key);
	if (node == NULL)
		return false;
	*k = node->key;
	*v = node->value;

	// search for removed node
	if (node->left == NULL || node->right == NULL) {
		rnode = node;
	} else {
		void *tmp;

		if (node->balance & BAL_POS) {
			rnode = node->right;
			while (rnode->left)
				rnode = rnode->left;
		} else {
			rnode = node->left;
			while (rnode->right)
				rnode = rnode->right;
		}

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
		goto out;
	}

	if (p->left == rnode) {
		p->left = cnode;
		dir = true;
	} else {
		assert(p->right == rnode);
		p->right = cnode;
		dir = false;
	}
	ccl_hbtree_unlink_ftree(tree, cnode, p, dir);
out:
	ccl_hbnode_dealloc(rnode, k, v);
	tree->count--;
	return true;
}

bool ccl_hbtree_delete(ccl_hbtree *tree, void *key)
{
	void *k, *v;

	if (!ccl_hbtree_unlink(tree, key, &k, &v))
		return false;
	if (tree->kfree != NULL)
		tree->kfree(k);
	if (tree->vfree != NULL)
		tree->vfree(v);
	return true;
}

static ccl_hbnode *ccl_hbnode_next(ccl_hbnode *node)
{
	ccl_hbnode *n;

	if (node->right) {
		n = node->right;
		while (n->left)
			n = n->left;
	} else {
		ccl_hbnode *p;

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

bool ccl_hbtree_foreach(ccl_hbtree *tree, ccl_dforeach_cb cb, void *user)
{
	ccl_hbnode *node;

	if (tree->root == NULL)
		return true;
	node = tree->root;
	while (node->left)
		node = node->left;
	for (; node != NULL; node = ccl_hbnode_next(node)) {
		if (!cb(node->key, node->value, user))
			return false;
	}
	return true;
}

static struct ccl_map_ops map_ops = {
	(ccl_map_free_cb)ccl_hbtree_free,
	(ccl_map_clear_cb)ccl_hbtree_clear,
	(ccl_map_select_cb)ccl_hbtree_select,
	(ccl_map_insert_cb)ccl_hbtree_insert,
	(ccl_map_delete_cb)ccl_hbtree_delete,
	(ccl_map_foreach_cb)ccl_hbtree_foreach,
};

ccl_map *ccl_smap_hbtree(ccl_cmp_cb cmp_cb, ccl_free_cb kfree_cb, ccl_free_cb vfree_cb)
{
	ccl_map *map;

	map = malloc(sizeof(*map));
	if (map == NULL)
		return NULL;
	map->obj = ccl_hbtree_new(cmp_cb, kfree_cb, vfree_cb);
	if (map->obj == NULL)
		goto err;
	map->ops = &map_ops;
	map->sorted = true;
	return map;
err:
	free(map);
	return NULL;
}
