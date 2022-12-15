# Copyright (C) 2022 Sergey V. Kostyuk
#
# This file is part of libclassic.
#
# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <https://www.gnu.org/licenses/>. 

AM_CFLAGS = \
	$(WARN_CFLAGS) \
	-I$(srcdir)/../include/ \
	-Wall

lib_LTLIBRARIES = libclassic.la

COBJECTS = map.c list.c vector.c \
	rb_tree.c hb_tree.c pr_tree.c \
	sp_tree.c tr_tree.c wb_tree.c \
	hashtable1.c hashtable2.c hashtable.c \
	skiplist.c

libclassic_la_SOURCES = $(COBJECTS)

libclassic_la_LDFLAGS = -export-symbols-regex '^(ccl_).*'
