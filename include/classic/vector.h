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

#include <stdbool.h>

#include <classic/common.h>

struct ccl_vector_t;

typedef struct ccl_vector_t {
	void **data;
	ccl_cmp_cb cmp;
	ccl_free_cb free;
	size_t count;
	size_t capacity;
	bool sorted;
} ccl_vector;

ccl_vector *ccl_vector_new(ccl_cmp_cb, ccl_free_cb);
void ccl_vector_clear(ccl_vector *);
void ccl_vector_free(ccl_vector *);
int ccl_vector_select_range(ccl_vector *, size_t, size_t, void *);
int ccl_vector_insert_range(ccl_vector *, size_t, size_t, void *);
int ccl_vector_update_range(ccl_vector *, size_t, size_t, void *);
int ccl_vector_unlink_range(ccl_vector *, size_t, size_t, void *);
int ccl_vector_delete_range(ccl_vector *, size_t, size_t);
int ccl_vector_foreach(ccl_vector *, ccl_foreach_cb, void *);
int ccl_vector_sort(ccl_vector *);

#define ccl_vector_select(vec,i,v)	ccl_vector_select_range((vec), (i), 1, (v))
#define ccl_vector_insert(vec,i,v)	ccl_vector_insert_range((vec), (i), 1, (v))
#define ccl_vector_update(vec,i,v)	ccl_vector_update_range((vec), (i), 1, (v))
#define ccl_vector_unlink(vec,i,v)	ccl_vector_unlink_range((vec), (i), 1, (v))
#define ccl_vector_delete(vec,i,v)	ccl_vector_delete_range((vec), (i), 1)

#endif
