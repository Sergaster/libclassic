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

#include <stdlib.h>
#include <string.h>

#include <classic/list.h>

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

	list = calloc(1, sizeof(*list));
	if (list == NULL)
		return NULL;
	ccl_list_init(list, cmp_cb, vfree_cb);
        return list;
}

void ccl_list_clear(ccl_list *list)
{
	ccl_list_iter *it, *next;

	it = list->head;
	while (it) {
		next = it->next;
		if (it->value != NULL && list->vfree)
			list->vfree(it->value);
		ccl_list_iter_free(it);
		it = next;
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

ccl_list_iter *ccl_list_prepend(ccl_list *list, void *v)
{       
	ccl_list_iter *it;
        
	it = ccl_list_iter_new(v);
	if (it == NULL)
		return NULL;
	ccl_list_iter_prepend(list, it);
	return it;
}

ccl_list_iter *ccl_list_append(ccl_list *list, void *v)
{
	ccl_list_iter *it;

	it = ccl_list_iter_new(v);
	if (it == NULL)
		return NULL;
	ccl_list_iter_append(list, it);
	return it;
}

ccl_list_iter *ccl_list_insert(ccl_list *list, void *v, void **pv)
{
	ccl_list_iter *it;

	it = ccl_list_iter_new(v);
	if (it == NULL)
		return NULL;
	*pv = &it->value;
	ccl_list_iter_insert(list, it);
	return it;
}

int ccl_list_insertion_sort(ccl_list *list)
{
	ccl_list_iter *it, *it2;
	void *v;

	if (list->cmp == NULL)
		return -1;
	if (list->sorted)
		return 0;
	for (it = list->head; it && it->value; it = it->next) {
		for (it2 = it->next; it2 && it2->value; it2 = it2->next) {
			if (list->cmp(it->value, it2->value) <= 0)
				continue;
			v = it->value;
			it->value = it2->value;
			it2->value = v;
		}
	}
	list->sorted = true;
	return 0;
}

int ccl_list_foreach(ccl_list *list, ccl_foreach_cb cb, void *user)
{
	ccl_list_iter *it;

	for (it = list->head; it; it = it->next) {
		if (cb(it->value, NULL, user))
			return -1;
	}
	return 0;
}

ccl_list_iter *ccl_list_search_iter(ccl_list *list, void *key)
{
	ccl_list_iter *it;
	void *q;
	int ret;

	if (list->cmp == NULL)
		return NULL;
	ccl_list_foreach_next(list, it, q) {
		ret = list->cmp(key, q);
		if (ret == 0)
			return it;
		else if (ret > 0 && list->sorted)
			return NULL;
	}
	return NULL;
}

ccl_list_iter *ccl_list_iter_new(void *v)
{
	ccl_list_iter *it;

        it = calloc(1, sizeof(*it));
	if (it == NULL)
		return NULL;
	it->prev = NULL;
	it->next = NULL;
	it->value = v;
	it->list = NULL;
	return it;
}

void ccl_list_iter_free(ccl_list_iter *it)
{       
	free(it);
	return;
}

void ccl_list_iter_prepend(ccl_list *dst, ccl_list_iter *it)
{
	if (dst->head)
		dst->head->prev = it;
	it->next = dst->head;
	it->prev = NULL;
	dst->head = it;
	if (dst->tail == NULL)
		dst->tail = it;
	dst->count++;
	dst->sorted = false;
	it->list = dst;
	return;
}

void ccl_list_iter_append(ccl_list *dst, ccl_list_iter *it)
{
	if (dst->tail)
		dst->tail->next = it;
	it->prev = dst->tail;
	it->next = NULL;
	dst->tail = it;
	if (dst->head == NULL)
		dst->head = it;
	dst->count++;
	dst->sorted = false;
	return;
}

void ccl_list_iter_insert(ccl_list *dst, ccl_list_iter *it)
{
	       
	ccl_list_iter *it2;

	if (dst->cmp == NULL || !dst->sorted) {
		ccl_list_iter_prepend(dst, it);
		return;
	}

	if (dst->head == NULL) {
		dst->head = it;
		dst->tail = it;
		goto out;
	}

	it2 = dst->head;
	while (it2->next != NULL) {
		if (dst->cmp(it->value, it2->value) >= 0)
			break;
		it2 = it2->next;
	}

	it->prev = it2;
	if (it2->next) {
		it->next = it2->next;
		it2->next->prev = it;
	} else {
		dst->tail = it;
	}
	it2->next = it;
out:
	dst->count++;
	it->list = dst;
	return;
}

void ccl_list_iter_unlink(ccl_list_iter *it)
{
	ccl_list *list = it->list;

	if (list->head == it)
		list->head = it->next;
	if (list->tail == it)
		list->tail = it->prev;
	if (it->prev)
		it->prev->next = it->next;
	if (it->next)
		it->next->prev = it->prev;
	list->count--;
	it->next = it->prev = NULL;
	it->list = NULL;
	return;
}
