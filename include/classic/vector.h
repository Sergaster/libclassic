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

#ifndef CCL_VECTOR_H
#define CCL_VECTOR_H

#include <stdlib.h>
#include <stdbool.h>

#include <classic/common.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef void ccl_vector;
typedef void ccl_vector_iter;

ccl_vector *ccl_vector_new(ccl_cmp_cb, ccl_free_cb);
void ccl_vector_init(ccl_vector *, ccl_cmp_cb, ccl_free_cb);
void ccl_vector_free(ccl_vector *);
void ccl_vector_clear(ccl_vector *);
bool ccl_vector_selectn(ccl_vector *, size_t, size_t, void **);
bool ccl_vector_select(ccl_vector *, size_t, void **);
bool ccl_vector_insertn(ccl_vector *, size_t, size_t, void **);
bool ccl_vector_insert(ccl_vector *, size_t, void *);
bool ccl_vector_pushn(ccl_vector *, size_t, void **);
bool ccl_vector_push(ccl_vector *, void *);
bool ccl_vector_updaten(ccl_vector *, size_t, size_t, void **);
bool ccl_vector_update(ccl_vector *, size_t, void *);
bool ccl_vector_unlinkn(ccl_vector *, size_t, size_t, void **);
bool ccl_vector_unlink(ccl_vector *, size_t, void **);
bool ccl_vector_deleten(ccl_vector *, size_t, size_t);
bool ccl_vector_delete(ccl_vector *, size_t);
bool ccl_vector_foreach(ccl_vector *, ccl_sforeach_cb, void *);
bool ccl_vector_sort(ccl_vector *);
size_t ccl_vector_count(ccl_vector *);
bool ccl_vector_sorted(ccl_vector *);

ccl_vector_iter *ccl_vector_iter_new(ccl_vector *);
void ccl_vector_iter_free(ccl_vector_iter *);
void ccl_vector_iter_begin(ccl_vector_iter *);
void ccl_vector_iter_end(ccl_vector_iter *);
bool ccl_vector_iter_value(ccl_vector_iter *, void **);
bool ccl_vector_iter_prev(ccl_vector_iter *);
bool ccl_vector_iter_prevn(ccl_vector_iter *, size_t);
bool ccl_vector_iter_next(ccl_vector_iter *);
bool ccl_vector_iter_nextn(ccl_vector_iter *, size_t);
bool ccl_vector_iter_unlink(ccl_vector_iter *, void **);
bool ccl_vector_iter_unlinkn(ccl_vector_iter *, size_t, void **);
bool ccl_vector_iter_delete(ccl_vector_iter *);
bool ccl_vector_iter_deleten(ccl_vector_iter *, size_t);
bool ccl_vector_iter_insert(ccl_vector_iter *, void *);
bool ccl_vector_iter_insertn(ccl_vector_iter *, size_t, void **);
int ccl_vector_iter_cmp(void *, ccl_vector_iter *);

#ifdef  __cplusplus
}
#endif

#endif
