/*
   Copyright (C) 2022 Sergey V. Kostyuk

   This file is part of libclassic.
   Author: Sergey V. Kostyuk <kostyuk.sergey79@gmail.com>
   Algo: skiplist implementation.
   Ref: [Pugh 1990], [Sedgewick 1998].

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

#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <classic/skiplist.h>

static ccl_skipnode *ccl_skipnode_alloc(void *k, unsigned link_count)
{
	ccl_skipnode *node;

	node = malloc(sizeof(*node) + sizeof(node->link[0]) * link_count);
	if (node == NULL)
		return NULL;
	node->key = k;
	node->value = NULL;
	node->prev = NULL;
	node->link_count = link_count;
	memset(node->link, 0, sizeof(node->link[0]) * link_count);
	return node;
}

static void ccl_skipnode_dealloc(ccl_skipnode *node, void **k, void **v)
{
	*k = node->key;
	*v = node->value;
	free(node);
	return;
}

ccl_skiplist *ccl_skiplist_new(ccl_cmp_cb cmp_cb, ccl_free_cb kfree_cb, ccl_free_cb vfree_cb, ccl_maxlink_cb maxlink_cb, unsigned max_link)
{
	ccl_skiplist *list;

	if (cmp_cb == NULL || maxlink_cb == NULL)
		return NULL;
	if (max_link > MAX_LINK)
		max_link = MAX_LINK;
	list = malloc(sizeof(*list));
	if (list == NULL)
		return NULL;
	list->head = ccl_skipnode_alloc(NULL, max_link);
	if (list->head == NULL)
		goto err;
	list->max_link = max_link;
	list->top_link = 0;
	list->cmp = cmp_cb;
	list->kfree = kfree_cb;
	list->vfree = vfree_cb;
	list->maxlink = maxlink_cb;
	list->count = 0;
	return list;
err:
	free(list);
	return NULL;
}

size_t ccl_skiplist_clear(ccl_skiplist *list)
{
	ccl_skipnode *node, *next;
	void *k, *v;
	size_t count;

	node = list->head->link[0];
	while (node) {
		next = node->link[0];
		ccl_skipnode_dealloc(node, &k, &v);
		if (list->kfree != NULL)
			list->kfree(k);
		if (v != NULL && list->vfree != NULL)
			list->vfree(v);
		node = next;
	}

	list->head->link[list->top_link] = NULL;
	count = 0;
	while (list->top_link) {
		list->head->link[--list->top_link] = NULL;
		count++;
	}
	return count;
}

void ccl_skiplist_free(ccl_skiplist *list)
{
	ccl_skiplist_clear(list);
	free(list->head);
	free(list);
	return;
}

static ccl_skipnode *ccl_skiplist_search_node(ccl_skiplist *list, void *k)
{
	ccl_skipnode *node1, *node2;
	unsigned i;
	int ret;

	node1 = list->head;
	for (i = list->top_link + 1; i-- > 0; ) {
		for (;;) {
			node2 = node1->link[i];
			if (node2 == NULL)
				break;
			ret = list->cmp(k, node2->key);
			if (ret < 0) {
				while (i > 0 && node1->link[i - 1] == node2)
					i--;
				break;
			} else if (ret == 0) {
				return node2;
			}
			node1 = node2;
		}
	}
	return NULL;
}

int ccl_skiplist_select(ccl_skiplist *list, void *k, void **v)
{
	ccl_skipnode *node;

	if (k == NULL)
		return -1;
	node = ccl_skiplist_search_node(list, k);
	if (node == NULL)
		return -1;
	*v = node->value;
	return 0;
}

int ccl_skiplist_insert(ccl_skiplist *list, void *k, void *v, void **pv)
{
	ccl_skipnode *node, *node1, *node2, *update[MAX_LINK];
	unsigned i, nlinks;
	int ret;
	
	*pv = NULL;
	if (k == NULL)
		return -1;
	memset(update, 0, MAX_LINK * sizeof(update[0]));

	node1 = list->head;
	for (i = list->top_link + 1; i-- > 0; ) {
		for (;;) {
			node2 = node1->link[i];
			if (node2 == NULL)
				break;
			ret = list->cmp(k, node2->key);
			if (ret < 0) {
				while (i > 0 && node1->link[i - 1] == node2)
					update[i--] = node1;
				break;
			} else if (ret == 0) {
				*pv = &node2->value;
				return -1;
			}
			node1 = node2;
		}
		update[i] = node1;
	}

	node = ccl_skipnode_alloc(k, list->maxlink(list));
	if (node == NULL)
		return -1;

	nlinks = node->link_count;
	if (list->top_link < nlinks) {
		for (i = list->top_link + 1; i <= nlinks; i++) {
			assert(update[i] == NULL);
			update[i] = list->head;
		}
		list->top_link = nlinks;
	}

	node->prev = (update[0] == list->head ? NULL : update[0]);
	if (update[0]->link[0])
		update[0]->link[0]->prev = node;
	for (i = 0; i < nlinks; i++) {
		assert(update[i]->link_count > i);
		node->link[i] = update[i]->link[i];
		update[i]->link[i] = node;
	}

	*pv = &node->value;
	list->count++;
	return 0;
}

int ccl_skiplist_unlink(ccl_skiplist *list, void *key, void **k, void **v)
{
	ccl_skipnode *node, *node2, *update[MAX_LINK];
	unsigned i;
	int ret;
	bool found;

	if (key == NULL)
		return -1;

	memset(update, 0, MAX_LINK * sizeof(update[0]));
	node = list->head;
	found = false;
	for (i = list->top_link + 1; i-- > 0;) {
		assert(node->link_count > i);
		for (;;) {
			node2 = node->link[i];
			if (node2 == NULL)
				break;
			ret = list->cmp(key, node2->key);
			if (ret <= 0) {
				while (i > 0 && node->link[i - 1] == node2)
					update[i--] = node;
				if (ret == 0)
					found = true;
				break;
			}
			node = node2;
		}
		update[i] = node;
	}

	if (!found)
		return -1;

	node = node->link[0];
	for (i = 0; i <= list->top_link; i++) {
		assert(update[i] != NULL);
		assert(update[i]->link_count > i);
		if (update[i]->link[i] != node)
			break;
		update[i]->link[i] = node->link[i];
	}

	if (node->prev)
		node->prev->link[0] = node->link[0];
	if (node->link[0])
		node->link[0]->prev = node->prev;
	while (list->top_link > 0 && !list->head->link[list->top_link - 1])
		list->top_link--;
	ccl_skipnode_dealloc(node, k, v);
	list->count--;
	return 0;
}

int ccl_skiplist_delete(ccl_skiplist *list, void *key)
{
	void *k, *v;

	if (ccl_skiplist_unlink(list, key, &k, &v))
		return -1;
	if (list->kfree != NULL)
		list->kfree(k);
	if (v != NULL && list->vfree != NULL)
		list->vfree(v);
	return 0;
}

int ccl_skiplist_foreach(ccl_skiplist *list, ccl_dforeach_cb cb, void *user)
{
	ccl_skipnode *node;

	for (node = list->head->link[0]; node; node = node->link[0]) {
		if (cb(node->key, node->value, user))
			return -1;
	}
	return 0;
}

static struct ccl_map_ops map_ops = {
	(ccl_map_free_cb)ccl_skiplist_free,
	(ccl_map_clear_cb)ccl_skiplist_clear,
	(ccl_map_select_cb)ccl_skiplist_select,
	(ccl_map_insert_cb)ccl_skiplist_insert,
	(ccl_map_delete_cb)ccl_skiplist_delete,
	(ccl_map_foreach_cb)ccl_skiplist_foreach,
};

ccl_map *ccl_map_skiplist(ccl_cmp_cb cmp_cb, ccl_free_cb kfree_cb, ccl_free_cb vfree_cb, ccl_maxlink_cb maxlink_cb, unsigned max_link)
{
	ccl_map *map;

	map = malloc(sizeof(*map));
	if (map == NULL)
		return NULL;
	map->obj = ccl_skiplist_new(cmp_cb, kfree_cb, vfree_cb, maxlink_cb, max_link);
	if (map->obj == NULL)
		goto err;
	map->ops = &map_ops;
	map->sorted = true;
	return map;
err:
	free(map);
	return NULL;
}
