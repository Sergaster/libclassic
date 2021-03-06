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

#include <classic/vector.h>

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
	void *v;
	size_t i;

	if (vec->data == NULL)
		return;
	if (vec->free) {
		for (i = 0; i < vec->count; i++) {
			v = vec->data[i];
			if (v != NULL)
				vec->free(v);
		}
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


int ccl_vector_select_range(ccl_vector *vec, size_t index, size_t count, void **into)
{
	if (index + count > vec->count)
		return -1;
	memcpy(into, &vec->data[index], count * sizeof(void *));
	return 0;
}

int ccl_vector_select(ccl_vector *vec, size_t index, void **into)
{
	void *v[1];

	if (ccl_vector_select_range(vec, index, 1, v))
		return -1;
	*into = *v;
	return 0;
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

int ccl_vector_insert_range(ccl_vector *vec, size_t index, size_t count, void **first)
{
	if (count == 0)
		return 0;
	if (index > vec->count)
		return -1;
	if (vec->count + count > vec->capacity) {
		void **data;
		size_t new_capacity;
		new_capacity = CCL_MAX(NEXT_VECTOR_CAPACITY, vec->count + count);
		data = calloc(new_capacity, sizeof(void *));
		if (data == NULL)
			return -1;
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
	return 0;
}

int ccl_vector_insert(ccl_vector *vec, size_t index, void *from)
{
	void *v[1];

	v[0] = from;
	if (ccl_vector_insert_range(vec, index, 1, v))
		return -1;
	return 0;
}

int ccl_vector_update_range(ccl_vector *vec, size_t index, size_t count, void **first)
{
	void *v;
	size_t i;

	if (count == 0)
		return 0;
	if (index + count > vec->count)
		return -1;
	if (vec->free) {
		for (i = 0; i < count; i++) {
			v = vec->data[index + i];
			if (v != NULL)
				vec->free(v);
		}
	}
	memcpy(&vec->data[index], first, count * sizeof(void *));
	vec->count += count;
	vec->sorted = false;
	return 0;
}

int ccl_vector_update(ccl_vector *vec, size_t index, void *from)
{
	void *v[1];

	v[0] = from;
	if (ccl_vector_update_range(vec, index, 1, v))
		return -1;
	return 0;
}

int ccl_vector_unlink_range(ccl_vector *vec, size_t index, size_t count, void **into)
{
	if (count == 0)
		return 0;
	if (index + count > vec->count)
		return -1;
	memcpy(into, &vec->data[index], count);
	memmove(&vec->data[index], &vec->data[index + count], count * sizeof(void *));
	vec -= count;
	return 0;
}

int ccl_vector_unlink(ccl_vector *vec, size_t index, void **into)
{
	void *v[1];

	if (ccl_vector_unlink_range(vec, index, 1, v))
		return -1;
	*into = v[0];
	return 0;
}

int ccl_vector_delete_range(ccl_vector *vec, size_t index, size_t count)
{
	void *v;
	size_t i;

	if (count == 0)
		return 0;
	if (index + count > vec->count)
		return -1;
	if (vec->free) {
		for (i = 0; i < count; i++) {
			v = vec->data[i];
			if (v != NULL)
				vec->free(v);
		}
	}
	memmove(&vec->data[index], &vec->data[index + count], count * sizeof(void *));
	vec -= count;
	return 0;
}

int ccl_vector_delete(ccl_vector *vec, size_t index)
{
	if (ccl_vector_delete_range(vec, index, 1))
		return -1;
	return 0;
}

int ccl_vector_foreach(ccl_vector *vec, ccl_sforeach_cb cb, void *user)
{
	int i;

	for (i = 0; i < vec->count; i++) {
		if (cb(vec->data[i], user))
			return -1;
	}
	return 0;
}

int ccl_vector_sort(ccl_vector *vec)
{
	if (vec->cmp == NULL)
		return -1;
	if (vec->sorted)
		return 0;
	qsort(vec->data, vec->count, sizeof(void *), vec->cmp);
	vec->sorted = true;
	return 0;
}
