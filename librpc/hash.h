/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

#ifndef LIBRPC_HASH_H
#define LIBRPC_HASH_H

#include <stdint.h>

struct hash_table;

enum hash_table_type {
	HASH_KEY_STR,		/* hash table maps 'const char *', using string hash function, to 'void *' */
	HASH_KEY_INT,		/* hash table maps an 'int' contained in a 'void *', using integer hash function, to 'void *' */
};

/* copied from glib */
#define H_POINTER_TO_INT(p)	((uintptr_t) (p))
#define H_INT_TO_POINTER(i)	((void *)(uintptr_t)(i))

struct hash_table *hash_table_new(enum hash_table_type t);

/* returns 1 if key has been inserted, 0 if not */
int hash_table_insert(struct hash_table *ht, void *key, void *value);

/* returns the value mapped to key, NULL if not found */
void *hash_table_search(struct hash_table *ht, void *key);

/* returns 1 if key was removed, 0 if not */
/* if p_value is not NULL, it will contains the value that was mapped to the removed key */
int hash_table_remove(struct hash_table *ht, void *key, void **p_value);

#endif
