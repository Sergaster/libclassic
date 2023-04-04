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

#include "vector.h"

#define CCL_MAX(x, y) (((x) > (y)) ? (x) : (y))

void ccl_vector_init(ccl_vector *vec, ccl_cmp_cb cmp_cb, ccl_free_cb free_cb)
{
	vec->data = NULL;
	vec->count = vec->capacity = 0;
	vec->free = free_cb;
	vec->sorted = true;
	return;
}

ccl_vector *ccl_vector_new(ccl_cmp_cb cmp_cb, ccl_free_cb free_cb)
{
	ccl_vector *vec;

	vec = malloc(sizeof(*vec));
	if (vec == NULL)
		return vec;
	ccl_vector_init(vec, cmp_cb, free_cb);
	return vec;
}

void ccl_vector_clear(ccl_vector *vec)
{
	size_t i;

	if (vec->data == NULL)
		return;
	if (vec->free) {
		for (i = 0; i < vec->count; i++)
			vec->free(vec->data[i]);
	}
	free(vec->data);
	vec->data = NULL;
	vec->count = 0;
	vec->capacity = 0;
	return;
}

void ccl_vector_free(ccl_vector *vec)
{
	ccl_vector_clear(vec);
	free(vec);
	return;
}


bool ccl_vector_selectn(ccl_vector *vec, size_t index, size_t count, void **into)
{
	if (index + count > vec->count)
		return false;
	memcpy(into, &vec->data[index], count * sizeof(void *));
	return true;
}

bool ccl_vector_select(ccl_vector *vec, size_t index, void **into)
{
	void *v[1];

	if (!ccl_vector_selectn(vec, index, 1, v))
		return false;
	*into = *v;
	return true;
}

#if __WORDSIZE == 32
// Chunk size 24, minus 4 (chunk header), minus 8 for capacity and len, 12 bytes remaining for 3 void *
#define INITIAL_VECTOR_LEN 3
#else
// For __WORDSIZE == 64
// Chunk size 48, minus 8 (chunk header), minus 8 for capacity and len, 32 bytes remaining for 4 void *
#define INITIAL_VECTOR_LEN 4
#endif

#define NEXT_VECTOR_CAPACITY (vec->capacity < INITIAL_VECTOR_LEN \
                ? INITIAL_VECTOR_LEN \
                : vec->capacity <= 12 ? vec->capacity * 2 \
                                      : vec->capacity + (vec->capacity >> 1))

bool ccl_vector_insertn(ccl_vector *vec, size_t index, size_t count, void **first)
{
	if (count == 0)
		return true;
	if (index > vec->count)
		return false;
	if (vec->count + count > vec->capacity) {
		void **data;
		size_t new_capacity;
		new_capacity = CCL_MAX(NEXT_VECTOR_CAPACITY, vec->count + count);
		data = calloc(new_capacity, sizeof(void *));
		if (data == NULL)
			return false;
		if (vec->data) {
			if (vec->count > 0)
				memcpy(data, vec->data, vec->count * sizeof(void *));
			free(vec->data);
		}
		vec->data = data;
		vec->capacity = new_capacity;
	}
	if (index != vec->count)
		memmove(&vec->data[index + count], &vec->data[index], vec->count - index);
	memcpy(&vec->data[index], first, count * sizeof(void *));
	vec->count += count;
	vec->sorted = false;
	return true;
}

bool ccl_vector_insert(ccl_vector *vec, size_t index, void *from)
{
	void *v[1];

	v[0] = from;
	if (!ccl_vector_insertn(vec, index, 1, v))
		return false;
	return true;
}

bool ccl_vector_pushn(ccl_vector *vec, size_t count, void **first)
{
	return ccl_vector_insertn(vec, vec->count, count, first);
}

bool ccl_vector_push(ccl_vector *vec, void *from)
{
	return ccl_vector_insert(vec, vec->count, from);
}

bool ccl_vector_updaten(ccl_vector *vec, size_t index, size_t count, void **first)
{
	size_t i;

	if (count == 0)
		return true;
	if (index + count > vec->count)
		return false;
	if (vec->free) {
		for (i = 0; i < count; i++)
			vec->free(vec->data[index + i]);
	}
	memcpy(&vec->data[index], first, count * sizeof(void *));
	vec->count += count;
	vec->sorted = false;
	return true;
}

bool ccl_vector_update(ccl_vector *vec, size_t index, void *from)
{
	void *v[1];

	v[0] = from;
	if (!ccl_vector_updaten(vec, index, 1, v))
		return false;
	return true;
}

bool ccl_vector_unlinkn(ccl_vector *vec, size_t index, size_t count, void **into)
{
	if (count == 0)
		return false;
	if (index + count > vec->count)
		return false;
	memcpy(into, &vec->data[index], count);
	memmove(&vec->data[index], &vec->data[index + count], count * sizeof(void *));
	vec->count -= count;
	return true;
}

bool ccl_vector_unlink(ccl_vector *vec, size_t index, void **into)
{
	void *v[1];

	if (!ccl_vector_unlinkn(vec, index, 1, v))
		return false;
	*into = v[0];
	return true;
}

bool ccl_vector_deleten(ccl_vector *vec, size_t index, size_t count)
{
	size_t i;

	if (count == 0)
		return true;
	if (index + count > vec->count)
		return false;
	if (vec->free) {
		for (i = 0; i < count; i++)
			vec->free(vec->data[index + i]);
	}
	memmove(&vec->data[index], &vec->data[index + count], count * sizeof(void *));
	vec->count -= count;
	return true;
}

bool ccl_vector_delete(ccl_vector *vec, size_t index)
{
	if (!ccl_vector_deleten(vec, index, 1))
		return false;
	return true;
}

bool ccl_vector_push_tail(ccl_vector *vec, void *v)
{
	return ccl_vector_insert(vec, vec->count, v);
}

bool ccl_vector_pop_tail(ccl_vector *vec, void **v)
{
	return ccl_vector_unlink(vec, vec->count - 1, v);
}

bool ccl_vector_push_head(ccl_vector *vec, void *v)
{
	return ccl_vector_insert(vec, 0, v);
}

bool ccl_vector_pop_head(ccl_vector *vec, void **v)
{
	return ccl_vector_unlink(vec, 0, v);
}

bool ccl_vector_foreach(ccl_vector *vec, ccl_sforeach_cb cb, void *user)
{
	int i;

	for (i = 0; i < vec->count; i++) {
		if (!cb(vec->data[i], user))
			return false;
	}
	return true;
}

bool ccl_vector_sort(ccl_vector *vec)
{
	if (vec->cmp == NULL)
		return false;
	if (vec->sorted)
		return true;
	qsort(vec->data, vec->count, sizeof(void *), vec->cmp);
	vec->sorted = true;
	return true;
}

size_t ccl_vector_count(ccl_vector *vec)
{
	return vec->count;
}

bool ccl_vector_sorted(ccl_vector *vec)
{
	return vec->sorted;
}

ccl_vector_iter *ccl_vector_iter_new(ccl_vector *vec)
{       
	ccl_vector_iter *it;

	it = malloc(sizeof(*it));
	if (it == NULL)
		return NULL;
	it->index = 0;
	it->vec = vec;
	return it;
}       

void ccl_vector_iter_free(ccl_vector_iter *it)
{       
	free(it);
	return;
}

void ccl_vector_iter_begin(ccl_vector_iter *it)
{
	it->index = 0;
	return;
}

void ccl_vector_iter_end(ccl_vector_iter *it)
{
	ccl_vector *vec = it->vec;
	if (vec->data == NULL || vec->count == 0)
		return;
	it->index = vec->count - 1;
	return;
}

bool ccl_vector_iter_value(ccl_vector_iter *it, void **v)
{
	ccl_vector *vec = it->vec;

	if (vec->data == NULL || vec->count == 0)
		return false;
	*v = vec->data[it->index];
	return true;
}

bool ccl_vector_iter_prevn(ccl_vector_iter *it, size_t n)
{
	ccl_vector *vec = it->vec;

	if (vec->data == NULL || vec->count == 0)
		return false;
	if (it->index + 1 > n)
		return false;
	it->index -= n;
	return true;
}

bool ccl_vector_iter_prev(ccl_vector_iter *it)
{
	return ccl_vector_iter_prevn(it, 1);
}

bool ccl_vector_iter_nextn(ccl_vector_iter *it, size_t n)
{       
	ccl_vector *vec = it->vec;

	if (vec->data == NULL || vec->count == 0)
		return false;
	if (it->index + n >= vec->count)
		return false;
	it->index += n;
	return true;
}

bool ccl_vector_iter_next(ccl_vector_iter *it)
{
        return ccl_vector_iter_nextn(it, 1);
}

bool ccl_vector_iter_unlinkn(ccl_vector_iter *it, size_t count, void **into)
{
	ccl_vector *vec = it->vec;

	if (vec->data == NULL || vec->count == 0)
		return false;
	return ccl_vector_unlinkn(vec, it->index, count, into);
}

bool ccl_vector_iter_unlink(ccl_vector_iter *it, void **into)
{
	ccl_vector *vec = it->vec;

	if (vec->data == NULL || vec->count == 0)
		return false;
	return ccl_vector_unlink(vec, it->index, into);
}

bool ccl_vector_iter_deleten(ccl_vector_iter *it, size_t count)
{
	ccl_vector *vec = it->vec;

	if (vec->data == NULL || vec->count == 0)
		return false;
	return ccl_vector_deleten(vec, it->index, count);
}

bool ccl_vector_iter_delete(ccl_vector_iter *it)
{
	return ccl_vector_iter_deleten(it, 1);
}

bool ccl_vector_iter_insertn(ccl_vector_iter *it, size_t count, void **first)
{
	ccl_vector *vec = it->vec;

	return ccl_vector_insertn(vec, it->index, count, first);
}

bool ccl_vector_iter_insert(ccl_vector_iter *it, void *from)
{
	return ccl_vector_iter_insertn(it, 1, from);
}

int ccl_vector_iter_cmp(void *v, ccl_vector_iter *it)
{
	ccl_vector *vec = it->vec;

	if (vec->cmp == NULL)
		return -1;
	return vec->cmp(v, vec->data[it->index]);
}
