/*
   Copyright (C) 2022 Sergey V. Kostyuk

   This file is part of libclassic.
   Author: Sergey V. Kostyuk <kostyuk.sergey79@gmail.com>
   Algo: chained hash-table, with chains sorted by hash
   Ref:  [Gonnet 1984], [Knuth 1998].

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

#include <classic/hashtable1.h>

#include "hashtable.h"

#define HT_ELEM_SIZE		sizeof(ccl_ht1_node)

static ccl_ht1_node *ccl_ht1_node_alloc(void *k, void *v, unsigned hash)
{
	ccl_ht1_node *node;

	node = malloc(sizeof(*node));
	if (node == NULL)
		return NULL;
	node->next = NULL;
	node->key = k;
	node->value = v;
	node->hash = hash;
	return node;
}

static void ccl_ht1_node_dealloc(ccl_ht1_node *node, void **k, void **v)
{
	*k = node->key;
	*v = node->value;
	free(node);
	return;
}

ccl_ht1 *ccl_ht1_new(ccl_cmp_cb cmp_cb, ccl_free_cb kfree_cb, ccl_free_cb vfree_cb, ccl_hash_cb hash_cb, unsigned size)
{
	ccl_ht1 *ht;

	if (cmp_cb == NULL || hash_cb == NULL)
		return NULL;
	ht = malloc(sizeof(*ht));
	if (ht == NULL)
		return NULL;
	ht->size = ccl_ht_prime_geq(size);
	ht->table = calloc(ht->size, HT_ELEM_SIZE);
	if (ht->table == NULL)
		goto err;
	memset(ht->table, 0, ht->size * HT_ELEM_SIZE);
	ht->cmp = cmp_cb;
	ht->kfree = kfree_cb;
	ht->vfree = vfree_cb;
	ht->hash = hash_cb;
	ht->count = 0;
	return ht;
err:
	free(ht);
	return NULL;
}

void ccl_ht1_clear(ccl_ht1 *ht)
{
	ccl_ht1_node *node, *next;
	void *k, *v;
	int i;

	for (i = 0; i < ht->size; i++) {
		node = ht->table[i];
		while (node != NULL) {
			next = node->next;
			ccl_ht1_node_dealloc(node, &k, &v);
			if (ht->kfree)
				ht->kfree(k);
			if (v != NULL && ht->vfree)
				ht->vfree(v);
			node = next;
		}
	}
	memset(ht->table, 0, ht->size * HT_ELEM_SIZE);
	ht->count = 0;
	return;
}

void ccl_ht1_free(ccl_ht1 *ht)
{
	ccl_ht1_clear(ht);
	free(ht->table);
	free(ht);
	return;
}

static ccl_ht1_node *ccl_ht1_search_node(ccl_ht1 *ht, void *k)
{
	ccl_ht1_node *node, *next;
	unsigned hn, hash;

	hash = ht->hash(k);
	hn = hash % ht->size;

	node = ht->table[hn];
	while (node != NULL) {
		next = node->next;
		if (hash > node->hash)
			return NULL;
		if (!ht->cmp(k, node->key))
			break;
		node = next;
	}
	return node;
}

bool ccl_ht1_select(ccl_ht1 *ht, void *k, void **v)
{
	ccl_ht1_node *node;

	if (k == NULL)
		return false;
	node = ccl_ht1_search_node(ht, k);
	if (node == NULL)
		return false;
	*v = node->value;
	return true;
}

static void ccl_ht1_transform(ccl_ht1 *ht, unsigned nsize)
{
	ccl_ht1_node *node, *node2, *prev2, *next, **table;
	unsigned hn;
	int i = 0;

	nsize = ccl_ht_prime_geq(nsize);
	if (nsize == ht->size)
		return;
	table = calloc(nsize, HT_ELEM_SIZE);
	if (table == NULL)	// hash table is unchanged
		return;

	for (i = 0; i < ht->size; i++) {
		node = ht->table[i];
		while (node != NULL) {
			next = node->next;

			hn = node->hash % nsize;

			if (table[hn] == NULL) {
				table[hn] = node;
				node->next = NULL;
				goto next;
			}

			node2 = table[hn];
			prev2 = NULL;
			while (node2 != NULL) {

				if (node->hash < node2->hash)
					break;
				prev2 = node2;
				node2 = node2->next;

			}

			if (node2 == NULL) {
				node->next = NULL;
				prev2->next = node;
				goto next;
			}

			if (prev2 == NULL) {
				node->next = node2;
				table[hn] = node;
			} else {
				node->next = node2;
				prev2->next = node;
			}
next:
			node = next;
		}
	}

	free(ht->table);
	ht->table = table;
	ht->size = nsize;
	return;
}

#define LOADFACTOR_NUMERATOR		2
#define LOADFACTOR_DENOMINATOR		3

bool ccl_ht1_insert(ccl_ht1 *ht, void *k, void *v, void **pv)
{
	ccl_ht1_node *node, *prev;
	unsigned hn, hash;

	*pv = NULL;
	if (k == NULL)
		return false;
	if (LOADFACTOR_DENOMINATOR * ht->count >= LOADFACTOR_NUMERATOR * ht->size)
		ccl_ht1_transform(ht, ht->size + 1);
	
	hash = ht->hash(k);
	hn = hash % ht->size;

	node = ht->table[hn];
	prev = NULL;
	while (node != NULL) {
		if (hash < node->hash)
			break;
		if (!ht->cmp(k, node->key)) {
			*pv = &node->value;
			return false;
		}
		prev = node;
		node = node->next;
	}

	node = ccl_ht1_node_alloc(k, v, hash);
	if (node == NULL)
		return false;
	if (ht->table[hn] == NULL) {
		ht->table[hn] = node;
		goto out;
	}

	if (prev == NULL) {
		node->next = ht->table[hn];
		ht->table[hn] = node;
	} else {
		node->next = prev->next;
		prev->next = node;
	}
out:
	*pv = &node->value;
	ht->count++;
	return true;
}

bool ccl_ht1_unlink(ccl_ht1 *ht, void *key, void **k, void **v)
{
	ccl_ht1_node *node, *prev;
	unsigned hn, hash;

	if (key == NULL)
		return false;
	hash = ht->hash(key);
	hn = hash % ht->size;

	node = ht->table[hn];
	prev = NULL;
	while (node != NULL) {
		if (hash < node->hash)
			return false;
		if (!ht->cmp(key, node->key)) {
			if (prev == NULL)
				ht->table[hn] = node->next;
			else
				prev->next = node->next;
			ccl_ht1_node_dealloc(node, k, v);
			ht->count--;
			return true;
		}
		prev = node;
		node = node->next;
	}
	return false;
}

bool ccl_ht1_delete(ccl_ht1 *ht, void *key)
{
	void *k, *v;

	if (!ccl_ht1_unlink(ht, key, &k, &v))
		return false;
	if (ht->kfree != NULL)
		ht->kfree(k);
	if (v != NULL && ht->vfree != NULL)
		ht->vfree(v);
        return true;
}

bool ccl_ht1_foreach(ccl_ht1 *ht, ccl_dforeach_cb cb, void *user)
{
	ccl_ht1_node *node;
	int i;

	for (i = 0; i < ht->size; i++) {
		node = ht->table[i];
		while (node != NULL) {
			if (!cb(node->key, node->value, user))
				return false;
			node = node->next;
		}
	}
	return true;
}

static struct ccl_map_ops map_ops = {
	(ccl_map_free_cb)ccl_ht1_free,
	(ccl_map_clear_cb)ccl_ht1_clear,
	(ccl_map_select_cb)ccl_ht1_select,
	(ccl_map_insert_cb)ccl_ht1_insert,
	(ccl_map_delete_cb)ccl_ht1_delete,
	(ccl_map_foreach_cb)ccl_ht1_foreach,
};

ccl_map *ccl_map_ht1(ccl_cmp_cb cmp_cb, ccl_free_cb kfree_cb, ccl_free_cb vfree_cb, ccl_hash_cb hash_cb, unsigned size)
{
	ccl_map *map;

	map = malloc(sizeof(*map));
	if (map == NULL)
		return NULL;
	map->obj = ccl_ht1_new(cmp_cb, kfree_cb, vfree_cb, hash_cb, size);
	if (map->obj == NULL)
		goto err;
	map->ops = &map_ops;
	map->sorted = false;
	return map;
err:
	free(map);
	return NULL;
}
