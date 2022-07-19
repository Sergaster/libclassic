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

#ifndef CCL_LIST_H
#define CCL_LIST_H

#include <stdlib.h>
#include <stdbool.h>

#include <classic/common.h>

struct ccl_list_t;

typedef struct ccl_list_iter_t {
	void *value;
	struct ccl_list_iter_t *prev;
	struct ccl_list_iter_t *next;
	struct ccl_list_t *list;
} ccl_list_iter;

typedef struct ccl_list_t {
	struct ccl_list_iter_t *head;
	struct ccl_list_iter_t *tail;
	ccl_free_cb vfree;
	ccl_cmp_cb cmp;
	size_t count;
	bool sorted;
} ccl_list;

#define ccl_list_foreach_next(list, it, pos) \
	for (it = list->head; it && (pos = it->value, 1); it = it->next)
/* Safe when calling ccl_list_iter_delete() while iterating over the list. */
#define ccl_list_foreach_safe(list, it, tmp, pos) \
	if (list) \
		for (it = list->head; it && (pos = it->data, tmp = it->next, 1); it = tmp)
#define ccl_list_foreach_prev(list, it, pos) \
	if (list) \
		for (it = list->tail; it && (pos = it->data, 1); it = it->prev)
#define ccl_list_foreach_prev_safe(list, it, tmp, pos) \
	for (it = list->tail; it && (pos = it->data, tmp = it->prev, 1); it = tmp)

#define ccl_list_iter_next(it)		(it)->next
#define ccl_list_iter_prev(it)		(it)->prev

ccl_list *ccl_list_new(ccl_cmp_cb cmp_cb, ccl_free_cb vfree_cb);
void ccl_list_free(ccl_list *list);
void ccl_list_init(ccl_list *list, ccl_cmp_cb cmp_cb, ccl_free_cb vfree_cb);
void ccl_list_clear(ccl_list *list);
ccl_list_iter *ccl_list_prepend(ccl_list *list, void *v);
ccl_list_iter *ccl_list_append(ccl_list *list, void *v);
ccl_list_iter *ccl_list_insert(ccl_list *list, void *v, void **);
int ccl_list_foreach(ccl_list *list, ccl_sforeach_cb cb, void *user);
int ccl_list_insertion_sort(ccl_list *list);
ccl_list_iter *ccl_list_push_head(ccl_list *list, void *v);
void *ccl_list_pop_head(ccl_list *list);
ccl_list_iter *ccl_list_push_tail(ccl_list *list, void *v);
void *ccl_list_pop_tail(ccl_list *list);

ccl_list_iter *ccl_list_iter_new(void *v);
void ccl_list_iter_free(ccl_list_iter *it);
void ccl_list_iter_prepend(ccl_list *dst, ccl_list_iter *it);
void ccl_list_iter_append(ccl_list *dst, ccl_list_iter *it);
void ccl_list_iter_insert(ccl_list *dst, ccl_list_iter *it);
void ccl_list_iter_unlink(ccl_list_iter *it);

#endif
