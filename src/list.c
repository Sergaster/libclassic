/*
   Copyright (C) 2022 Sergey V. Kostyuk

   This file is part of libclassic.
   Author: Sergey V. Kostyuk <kostyuk.sergey79@gmail.com>

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

#include "list.h"

static ccl_list_node *_ccl_list_node_new(void *v)
{
	ccl_list_node *node;

        node = malloc(sizeof(*node));
	if (node == NULL)
		return NULL;
	node->prev = NULL;
	node->next = NULL;
	node->value = v;
	return node;
}

static void _ccl_list_node_free(ccl_list_node *node)
{       
	free(node);
	return;
}

static void _ccl_list_node_prepend(ccl_list *list, ccl_list_node *node)
{
	if (list->head)
		list->head->prev = node;
	node->next = list->head;
	node->prev = NULL;
	list->head = node;
	if (list->tail == NULL)
		list->tail = node;
	list->count++;
	list->sorted = false;
	return;
}

static void _ccl_list_node_append(ccl_list *list, ccl_list_node *node)
{
	if (list->tail)
		list->tail->next = node;
	node->prev = list->tail;
	node->next = NULL;
	list->tail = node;
	if (list->head == NULL)
		list->head = node;
	list->count++;
	list->sorted = false;
	return;
}

void ccl_list_init(ccl_list *list, ccl_cmp_cb cmp_cb, ccl_free_cb vfree_cb)
{
	list->head = NULL;
	list->tail = NULL;
	list->cmp = cmp_cb;
	list->vfree = vfree_cb;
	list->count = 0;
	list->sorted = true;
        return;
}

ccl_list *ccl_list_new(ccl_cmp_cb cmp_cb, ccl_free_cb vfree_cb)
{
	ccl_list *list;

	list = malloc(sizeof(*list));
	if (list == NULL)
		return NULL;
	ccl_list_init(list, cmp_cb, vfree_cb);
        return list;
}

void ccl_list_clear(ccl_list *list)
{
	ccl_list_node *node, *next;

	node = list->head;
	while (node) {
		next = node->next;
		if (node->value != NULL && list->vfree)
			list->vfree(node->value);
		_ccl_list_node_free(node);
		node = next;
	}
	list->head = list->tail = NULL;
	list->count = 0;
	return;
}

void ccl_list_free(ccl_list *list)
{
	ccl_list_clear(list);
	free(list);
	return;
}

bool ccl_list_prepend(ccl_list *list, void *v)
{       
	ccl_list_node *node;
        
	node = _ccl_list_node_new(v);
	if (node == NULL)
		return false;
	_ccl_list_node_prepend(list, node);
	return true;
}

bool ccl_list_append(ccl_list *list, void *v)
{
	ccl_list_node *node;

	node = _ccl_list_node_new(v);
	if (node == NULL)
		return false;
	_ccl_list_node_append(list, node);
	return true;
}

bool ccl_list_push_tail(ccl_list *list, void *v)
{
	return ccl_list_append(list, v);
}

void *ccl_list_pop_tail(ccl_list *list)
{
	ccl_list_node *node;
	void *v;

	if (list->tail == NULL)
		return NULL;
	node = list->tail;
	v = node->value;
	if (list->head == list->tail) {
		list->head = list->tail = NULL;
	} else {
		list->tail = node->prev;
		list->tail->next = NULL;
	}
	list->count--;
	_ccl_list_node_free(node);
	return v;
}

bool ccl_list_push_head(ccl_list *list, void *v)
{
	return ccl_list_prepend(list, v);
}

void *ccl_list_pop_head(ccl_list *list)
{
	ccl_list_node *node;
	void *v;

	if (list->head == NULL)
		return NULL;
	node = list->head;
	v = node->value;
	if (list->head == list->tail) {
		list->head = list->tail = NULL;
	} else {
		list->head = node->next;
		list->head->prev = NULL;
	}
	list->count--;
	_ccl_list_node_free(node);
	return v;
}

bool ccl_list_foreach(ccl_list *list, ccl_sforeach_cb cb, void *user)
{
	ccl_list_node *node;

	for (node = list->head; node; node = node->next) {
		if (!cb(node->value, user))
			return false;
	}
	return true;
}

bool ccl_list_sort(ccl_list *list)
{
	ccl_list_node *node, *node2;
	void *v;

	if (list->cmp == NULL)
		return false;
	if (list->sorted)
		return true;
	for (node = list->head; node && node->value; node = node->next) {
		for (node2 = node->next; node2 && node2->value; node2 = node2->next) {
			if (list->cmp(node->value, node2->value) <= 0)
				continue;
			v = node->value;
			node->value = node2->value;
			node2->value = v;
		}
	}
	list->sorted = true;
	return true;
}

size_t ccl_list_count(ccl_list *list)
{
	return list->count;
}

ccl_list_iter *ccl_list_iter_new(ccl_list *list)
{
	ccl_list_iter *it;

	it = malloc(sizeof(*it));
	if (it == NULL)
		return NULL;
	it->node = list->head;
	it->list = list;
	return it;
}

void ccl_list_iter_free(ccl_list_iter *it)
{
	free(it);
	return;
}

void ccl_list_iter_begin(ccl_list_iter *it)
{
	it->node = it->list->head;
	return;
}

void ccl_list_iter_end(ccl_list_iter *it)
{
	it->node = it->list->tail;
	return;
}

bool ccl_list_iter_value(ccl_list_iter *it, void **v)
{
	if (it->node == NULL)
		return false;
	*v = it->node->value;
	return true;
}

bool ccl_list_iter_prevn(ccl_list_iter *it, size_t n)
{
	ccl_list_node *node = it->node;
	size_t i;

	for (i = 0; i < n; i++) {
		if (node == NULL)
			return false;
		node = node->prev;
	}
	it->node = node;
	return true;
}

bool ccl_list_iter_prev(ccl_list_iter *it)
{
	if (it->node == NULL || it->node->prev == NULL)
		return false;
	it->node = it->node->prev;
	return true;
}

bool ccl_list_iter_nextn(ccl_list_iter *it, size_t n)
{
	ccl_list_node *node = it->node;
	size_t i;

	for (i = 0; i < n; i++) {
		if (node == NULL)
			return false;
		node = node->next;
	}
	it->node = node;
	return true;
}

bool ccl_list_iter_next(ccl_list_iter *it)
{
	if (it->node == NULL || it->node->next == NULL)
		return false;
	it->node = it->node->next;
	return true;
}

void *ccl_list_iter_unlink(ccl_list_iter *it)
{
	ccl_list *list = it->list;
	ccl_list_node *node = it->node;

	if (list->head == node)
		list->head = node->next;
	if (list->tail == node)
		list->tail = node->prev;
	if (node->prev)
		node->prev->next = node->next;
	if (node->next)
		node->next->prev = node->prev;
	list->count--;
	node->next = node->prev = NULL;
	return node->value;
}

void ccl_list_iter_delete(ccl_list_iter *it)
{
	ccl_list *list = it->list;
	ccl_list_node *node = it->node;
	void *v;

	if (node == NULL)
		return;
	v = ccl_list_iter_unlink(it);
	if (list->vfree)
		list->vfree(v);
	ccl_list_iter_free(it);
	return;
}

bool ccl_list_iter_insert(ccl_list_iter *it, void *v)
{
	ccl_list *list = it->list;
	ccl_list_node *node, *node2;

	node = _ccl_list_node_new(v);
	if (node == NULL)
		return false;

	if (list->head == NULL) {
		list->head = node;
		list->tail = node;
		goto out;
	}

	node2 = it->node;
	if (node2 == NULL) {
		ccl_list_node *tail = list->tail;

		tail->next = node;
		node->prev = tail;
		list->tail = node;
	} else {
		if (node2->prev == NULL)
			list->head = node;
		node->prev = node2->prev;
		node->next = node2;
		node2->prev = node;
	}
out:
	list->count++;
	list->sorted = false;
	return true;
}

int ccl_list_iter_cmp(void *v, ccl_list_iter *it)
{
	ccl_list *list = it->list;

	if (it->node == NULL)
		return -1;
	return list->cmp(v, it->node->value);
}
