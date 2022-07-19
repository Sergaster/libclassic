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

#ifndef CCL_MAP_H
#define CCL_MAP_H

#include <stdlib.h>
#include <stdbool.h>

#include <classic/common.h>

typedef void		(* ccl_map_free_cb)(void *obj);
typedef size_t		(* ccl_map_clear_cb)(void *obj);
typedef int		(* ccl_map_select_cb)(void *obj, const void *k, void **v);
typedef int		(* ccl_map_insert_cb)(void *obj, const void *k, void *v, void **pv);
typedef int		(* ccl_map_delete_cb)(void *obj, const void *k);
typedef int		(* ccl_map_foreach_cb)(void *obj, ccl_dforeach_cb cb, void *user);

struct ccl_map_ops {
	ccl_map_free_cb	free;
	ccl_map_clear_cb	clear;
	ccl_map_select_cb	select;
	ccl_map_insert_cb	insert;
	ccl_map_delete_cb	delete;
	ccl_map_foreach_cb	foreach;
};


typedef struct ccl_map_t {
	void *obj;
	struct ccl_map_ops *ops;
	bool sorted;
} ccl_map;

void ccl_map_free(ccl_map *map);
#define ccl_map_clear(map)		(map)->ops->clear((map)->obj)
#define ccl_map_select(map,k,v)		(map)->ops->select((map)->obj, (k), (v))
#define ccl_map_insert(map,k,v,p)	(map)->ops->insert((map)->obj, (k), (v), (p))
#define ccl_map_delete(map,k)		(map)->ops->delete((map)->obj, (k))
#define ccl_map_foreach(map,cb,u)	(map)->ops->foreach((map)->obj, (cb), (u))
#define ccl_map_sorted(map)		(map)->sorted

#endif
