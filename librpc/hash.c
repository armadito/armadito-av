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
	size_t len;
	struct hash_table_entry *table;
};

struct hash_table *hash_table_new(size_t len)
{
	struct hash_table *ht;

	ht = malloc(sizeof(struct hash_table));
	ht->len = len;
	ht->table = calloc(len, sizeof(struct hash_table_entry));

	return ht;
}

/* PJW non-cryptographic hash function */
static uint32_t hash(const char *s)
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

int hash_table_insert(struct hash_table *ht, const char *key, void *value)
{
	size_t h, i, w;

	h = hash(key) % ht->len;

	for (i = 0; i < ht->len; i++) {
		w = (h + i) % ht->len;

		if (ht->table[w].key == NULL)
			break;
	}

	if (i == ht->len)
		return 1; /* no NULL place found => table is full */

	ht->table[w].key = strdup(key);
	ht->table[w].value = value;

	return 0;
}

void *hash_table_search(struct hash_table *ht, const char *key)
{
	size_t h, i, w;

	h = hash(key) % ht->len;

	for (i = 0; i < ht->len; i++) {
		w = (h + i) % ht->len;

		if (ht->table[w].key == NULL)
			return NULL;

		if (!strcmp(ht->table[w].key, key))
			return ht->table[w].value;
	}

	return NULL;
}
