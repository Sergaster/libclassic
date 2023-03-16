/*
   Copyright (C) 2022 Sergey V. Kostyuk

   This file is part of libclassic.
   Author: Sergey V. Kostyuk <kostyuk.sergey79@gmail.com>
   Algo: open-addressing hash-table interface.
   Ref: [Gonnet 1984], [Knuth 1998].

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

#include <classic/hashtable2.h>

#include "hashtable.h"

#define HT_ELEM_SIZE			sizeof(ccl_ht2_node)
#define ccl_ht2_ptr(ht,n)		&(ht)->table[n]

ccl_ht2 *ccl_ht2_new(ccl_cmp_cb cmp_cb, ccl_free_cb kfree_cb, ccl_free_cb vfree_cb, ccl_hash_cb hash_cb, unsigned int size)
{
	ccl_ht2 *ht;

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

void ccl_ht2_clear(ccl_ht2 *ht)
{
	ccl_ht2_node *node;
	int i;

	for (i = 0; i < ht->size; i++) {
		node = ccl_ht2_ptr(ht, i);
		if (node->key == NULL)
			continue;
		if (ht->kfree)
			ht->kfree(node->key);
		if (ht->vfree)
			ht->vfree(node->value);
		ht->count--;
	}
	return;
}

void ccl_ht2_free(ccl_ht2 *ht)
{
	ccl_ht2_clear(ht);
	free(ht->table);
	free(ht);
	return;
}

static ccl_ht2_node *ccl_ht2_search_node(ccl_ht2 *ht, void *k)
{
	ccl_ht2_node *node;
	unsigned hn, i, hash;

	if (k == NULL)
		return NULL;
	hash = ht->hash(k);
	hn = hash % ht->size;

	i = hn;
	do {
		node = ccl_ht2_ptr(ht, i);
		if (node->key == NULL)
			break;
		if (node->hash == hash && !ht->cmp(k, node->key))
			return node;
		i++;
		if (i == ht->size)
			i = 0;
	} while (i != hn);
	return NULL;
}

bool ccl_ht2_select(ccl_ht2 *ht, void *k, void **v)
{
	ccl_ht2_node *node;

	node = ccl_ht2_search_node(ht, k);
	if (node == NULL)
		return false;
	*v = node->value;
	return true;
}

static void ccl_ht2_transform(ccl_ht2 *ht, unsigned nsize)
{
	ccl_ht2_node *table;
	ccl_ht2_node *node, *node2;
	int i, j;
	unsigned hn;

	nsize = ccl_ht_prime_geq(nsize);
	if (nsize == ht->size)
		return;
	table = calloc(nsize, sizeof(*table));
	if (table == NULL)      // hash table is unchanged
		return;
	memset(table, 0, nsize * HT_ELEM_SIZE);
	
	for (i = 0; i < ht->size; ++i) {
		node = ccl_ht2_ptr(ht, i);
		if (node->key == NULL)
			continue;
		hn = node->hash % nsize;

		j = hn;
		do {
			node2 = &table[j];
			if (node2->key == NULL) {
				node2->key = node->key;
				node2->value = node->value;
				node2->hash = node->hash;
				break;
			}

			if (node->hash == node2->hash && !ht->cmp(node->key, node2->key)) {		// hash table is unchanged
				free(table);
				return;

			}
			j++;
			if (j == nsize)
				j = 0;
		} while(j != hn);
	}
	free(ht->table);
	ht->table = table;
	ht->size = nsize;
	return;
}

#define LOADFACTOR_NUMERATOR    2
#define LOADFACTOR_DENOMINATOR  3

bool ccl_ht2_insert(ccl_ht2 *ht, void *k, void *v, void **pv)
{
	ccl_ht2_node *node;
	unsigned hn, i, hash;

	*pv = NULL;
	if (k == NULL)
		return false;
	if (LOADFACTOR_DENOMINATOR * ht->count >= LOADFACTOR_NUMERATOR * ht->size)
		ccl_ht2_transform(ht, ht->size + 1);

	hash = ht->hash(k);
	hn = hash % ht->size;

	i = hn;
	do {
		node = ccl_ht2_ptr(ht, i);
		if (node->key == NULL) {
			node->key = k;
			node->value = v;
			node->hash = hash;
			ht->count++;
			*pv = &node->value;
			return true;
		}

		if (node->hash == hash && !ht->cmp(k, node->key)) {
			*pv = &node->value;
			return false;
		}
		i++;
		if (i == ht->size)
			i = 0;
	} while (i != hn);

	return false;
}

static void ccl_ht2_update_table(ccl_ht2 *ht, unsigned start, unsigned end)
{
	ccl_ht2_node *node, *node2, n;
	unsigned hn2, i, j;

	i = start;
	do {
		node = ccl_ht2_ptr(ht, i);

		if (node->key == NULL)
			return;
		n = *node;
		node->key = NULL;

		hn2 = n.hash % ht->size;
		j = hn2;
		do {
			node2 = ccl_ht2_ptr(ht, j);
			if (node2->key == NULL) {
				node2->key = n.key;
				node2->value = n.value;
				node2->hash = n.hash;
				break;
			}

			assert(node2->hash != n.hash || ht->cmp(n.key, node2->key));
			j++;
			if (j == ht->size)
				j = 0;
		} while (j != hn2);

		i++;
		if (i == ht->size)
			i = 0;
	} while (i != end);

	return;
}

bool ccl_ht2_unlink(ccl_ht2 *ht, void *key, void **k, void **v)
{
	ccl_ht2_node *node;
	unsigned hn, i, hash;

	if (key == NULL)
		return false;
	hash = ht->hash(key);
	hn = hash % ht->size;

	i = hn;

	do {
		node = ccl_ht2_ptr(ht, i);
		if (node->key == NULL)
			return false;
		if (node->hash == hash && !ht->cmp(key, node->key)) {
			*k = node->key;
			*v = node->value;
			node->key = NULL;
			ht->count--;
			i++;
			if (i == ht->size)
				i = 0;
			ccl_ht2_update_table(ht, i, hn);
			return true;
		}
		i++;
		if (i == ht->size)
			i = 0;
	} while (i != hn);

	return false;
}

bool ccl_ht2_delete(ccl_ht2 *ht, void *key)
{
	void *k, *v;

	if (!ccl_ht2_unlink(ht, key, &k, &v))
		return false;
	if (ht->kfree != NULL)
		ht->kfree(k);
	if (ht->vfree != NULL)
		ht->vfree(v);
        return true;
}

bool ccl_ht2_foreach(ccl_ht2 *ht, ccl_dforeach_cb cb, void *user)
{               
        ccl_ht2_node *node;
	int i;

	for (i = 0; i < ht->size; i++) {
		node = ccl_ht2_ptr(ht, i);
		if (node->key == NULL)
			continue;
		if (!cb(node->key, node->value, user))
			return false;
	}
	return true;
}

static struct ccl_map_ops map_ops = {
	(ccl_map_free_cb)ccl_ht2_free,
	(ccl_map_clear_cb)ccl_ht2_clear,
	(ccl_map_select_cb)ccl_ht2_select,
	(ccl_map_insert_cb)ccl_ht2_insert,
	(ccl_map_delete_cb)ccl_ht2_delete,
	(ccl_map_foreach_cb)ccl_ht2_foreach,
};

ccl_map *ccl_umap_ht2(ccl_cmp_cb cmp_cb, ccl_free_cb kfree_cb, ccl_free_cb vfree_cb, ccl_hash_cb hash_cb, unsigned size)
{
	ccl_map *map;

	map = malloc(sizeof(*map));
	if (map == NULL)
		return NULL;
	map->obj = ccl_ht2_new(cmp_cb, kfree_cb, vfree_cb, hash_cb, size);
	if (map->obj == NULL)
		goto err;
	map->ops = &map_ops;
	map->sorted = false;
	return map;
err:
	free(map);
	return NULL;
}
