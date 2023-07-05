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

#ifndef _CCL_VECTOR_H
#define _CCL_VECTOR_H

#include <stdlib.h>
#include <stdbool.h>

#include <classic/common.h>

typedef struct ccl_vector_t {
	void **data;
	ccl_cmp_cb cmp;
	ccl_free_cb free;
	size_t count;
	size_t capacity;
	bool sorted;
} ccl_vector;

typedef struct ccl_vector_iter_t {
	size_t index;
	ccl_vector *vec;
} ccl_vector_iter;

#endif
