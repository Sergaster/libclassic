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

#include "hashtable.h"

static const unsigned ccl_primes[] = {
	11,         17,         37,         67,         131,
	257,        521,        1031,       2053,       4099,
	8209,       16411,      32771,      65537,      131101,
	262147,     524309,     1048583,    2097169,    4194319,
	8388617,    16777259,   33554467,   67108879,   134217757,
	268435459,  536870923,  1073741827, 2147483659, 4294967291
};

static const unsigned ccl_num_primes = sizeof(ccl_primes) / sizeof(ccl_primes[0]);

unsigned ccl_ht_prime_geq(unsigned n)
{
	unsigned index;

	for (index = 0; index < ccl_num_primes; ++index) {
		if (ccl_primes[index] >= n)
			return ccl_primes[index];
	}
	return ccl_primes[ccl_num_primes - 1];
}
