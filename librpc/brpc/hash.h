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

#ifndef LIBJRPC_HASH_H
#define LIBJRPC_HASH_H

#include <stdint.h>

typedef void (*destroy_cb_t)(void *p);
typedef size_t (*hash_fun_t)(void *k);
typedef int (*equal_fun_t)(void *p, void *q);

struct hash_table;

size_t hash_int(void *k);
int equal_int(void *p, void *q);

size_t hash_pointer(void *k);
int equal_pointer(void *p, void *q);

size_t hash_str(void *k);
int equal_str(void *p, void *q);

/* copied from glib */
#define H_POINTER_TO_INT(p)	((uintptr_t) (p))
#define H_INT_TO_POINTER(i)	((void *)(uintptr_t)(i))

struct hash_table *hash_table_new(hash_fun_t hash_fun, equal_fun_t equal_fun, destroy_cb_t key_destroy_cb, destroy_cb_t value_destroy_cb);

void hash_table_free(struct hash_table *ht);

/* returns 1 if key has been inserted, 0 if not */
int hash_table_insert(struct hash_table *ht, void *key, void *value);

/* returns the value mapped to key, NULL if not found */
void *hash_table_search(struct hash_table *ht, void *key);

/* returns 1 if key was removed, 0 if not */
int hash_table_remove(struct hash_table *ht, void *key);

#endif
