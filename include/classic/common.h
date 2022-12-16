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

#ifndef CCL_COMMON_H
#define CCL_COMMON_H

#include <stdbool.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef void		(* ccl_free_cb)(void *);
typedef int		(* ccl_cmp_cb)(const void *, const void *);
typedef bool		(* ccl_sforeach_cb)(void *, void *);
typedef bool		(* ccl_dforeach_cb)(const void *, void *, void *);
typedef unsigned	(* ccl_hash_cb)(const void *);

#define container_of(ptr, type, member) ((type *)((char *)(__typeof__(((type *)0)->member) *){ ptr } - offsetof(type, member)))

#ifdef  __cplusplus
}
#endif

#endif
