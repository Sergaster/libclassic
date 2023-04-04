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

#ifdef  __cplusplus
extern "C" {
#endif

typedef void ccl_list;
typedef void ccl_list_iter;

ccl_list *ccl_list_new(ccl_cmp_cb, ccl_free_cb);
void ccl_list_free(ccl_list *);
void ccl_list_init(ccl_list *, ccl_cmp_cb, ccl_free_cb);
void ccl_list_clear(ccl_list *);
bool ccl_list_foreach(ccl_list *, ccl_sforeach_cb, void *);
bool ccl_list_push_head(ccl_list *l, void *);
bool ccl_list_pop_head(ccl_list *, void **);
bool ccl_list_push_tail(ccl_list *l, void *);
bool ccl_list_pop_tail(ccl_list *, void **);
bool ccl_list_sort(ccl_list *);
size_t ccl_list_count(ccl_list *);

ccl_list_iter *ccl_list_iter_new(ccl_list *);
void ccl_list_iter_free(ccl_list_iter *);
void ccl_list_iter_unlink(ccl_list_iter *);
void ccl_list_iter_delete(ccl_list_iter *);
bool ccl_list_iter_insert(ccl_list_iter *, void *);
bool ccl_list_iter_value(ccl_list_iter *, void **);
void ccl_list_iter_begin(ccl_list_iter *);
void ccl_list_iter_end(ccl_list_iter *);
bool ccl_list_iter_prev(ccl_list_iter *);
bool ccl_list_iter_prevn(ccl_list_iter *, size_t);
bool ccl_list_iter_next(ccl_list_iter *);
bool ccl_list_iter_nextn(ccl_list_iter *, size_t);
int ccl_list_iter_cmp(void *, ccl_list_iter *);

#ifdef  __cplusplus
}
#endif

#endif
