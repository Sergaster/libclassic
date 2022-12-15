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

#ifndef _CCL_LIST_H
#define _CCL_LIST_H

#include <stdlib.h>
#include <stdbool.h>

#include <classic/common.h>

typedef struct ccl_list_node_t {
	struct ccl_list_node_t *prev;
	struct ccl_list_node_t *next;
	void *value;
} ccl_list_node;

typedef struct ccl_list_t {
	struct ccl_list_node_t *head;
	struct ccl_list_node_t *tail;
	ccl_free_cb vfree;
	ccl_cmp_cb cmp;
	size_t count;
	bool sorted;
} ccl_list;

typedef struct ccl_list_iter_t {
	ccl_list_node *node;
	ccl_list *list;
} ccl_list_iter;

#endif
