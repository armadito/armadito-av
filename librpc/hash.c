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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"

struct hash_table_entry {
	const char *key;
	void *value;
};

struct hash_table {
	enum hash_table_type type;
	size_t size;
	struct hash_table_entry *table;
};

#define HASH_DEFAULT_SIZE 128

struct hash_table *hash_table_new(enum hash_table_type t)
{
	struct hash_table *ht;

	ht = malloc(sizeof(struct hash_table));
	ht->type = t;
	ht->size = HASH_DEFAULT_SIZE;
	ht->table = calloc(ht->size, sizeof(struct hash_table_entry));

	return ht;
}

/* PJW non-cryptographic string hash function */
static uint32_t hash_str(const char *s)
{
	uint32_t h = 0, high;
	const char *p = s;

	while (*p) {
		h = (h << 4) + *p++;
		if (high = h & 0xF0000000)
			h ^= high >> 24;
		h &= ~high;
	}

	return h;
}

static uint32_t hash_pointer(void *p)
{
	return 0;
}

#define HASH(HT, K) ((HT->type == HASH_KEY_STR) ? hash_str(K) : hash_pointer(K))
#define EQUAL(HT, K1, K2) ((HT->type == HASH_KEY_STR) ? (strcmp((const char *)K1, (const char *)K2) == 0) : (K1 == K2))

int hash_table_insert(struct hash_table *ht, void *key, void *value)
{
	size_t h, i, w;

	h = HASH(ht, key) % ht->size;

	for (i = 0; i < ht->size; i++) {
		w = (h + i) % ht->size;

		if (ht->table[w].key == NULL)
			break;
	}

	if (i == ht->size)
		return 0; /* no NULL place found => table is full */

	if (ht->type == HASH_KEY_STR)
		ht->table[w].key = strdup(key);
	else
		ht->table[w].key = key;
	ht->table[w].value = value;

	return 1;
}

void *hash_table_search(struct hash_table *ht, void *key)
{
	size_t h, i, w;

	h = HASH(ht, key) % ht->size;

	for (i = 0; i < ht->size; i++) {
		w = (h + i) % ht->size;

		if (ht->table[w].key == NULL)
			return NULL;

		if (EQUAL(ht, ht->table[w].key, key))
			return ht->table[w].value;
	}

	return NULL;
}
