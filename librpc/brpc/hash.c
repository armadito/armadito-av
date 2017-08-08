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

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"

struct hash_table_entry {
	void *key;
	void *value;
	enum {
		EMPTY = 0,
		REMOVED,
		OCCUPIED,
	} state;
};

#define IS_EMPTY(T, I) (T[(I)].state == EMPTY)
#define IS_REMOVED(T, I) (T[(I)].state == REMOVED)
#define IS_OCCUPIED(T, I) (T[(I)].state == OCCUPIED)

struct hash_table {
	size_t size;
	struct hash_table_entry *table;
	size_t key_count;
	hash_fun_t hash_fun;
	equal_fun_t equal_fun;
	destroy_cb_t key_destroy_cb;
	destroy_cb_t value_destroy_cb;
};

#define HASH_DEFAULT_SIZE 64

#define HASH(HT, K) (*HT->hash_fun)(K)
#define EQUAL(HT, P, Q) (*HT->equal_fun)(P, Q)

size_t hash_int(void *k)
{
	uintptr_t p = H_POINTER_TO_INT(k);

	/* for 64 bits, use 11400712997709160919 which is a prime close to 2^64 x phi */
	return p * UINT64_C(11400712997709160919);
}

int equal_int(void *p, void *q)
{
	return p == q;
}

size_t hash_pointer(void *k)
{
	uint64_t p = H_POINTER_TO_INT(k);

	p ^= p >> 33;
	p *= UINT64_C(0xff51afd7ed558ccd);
	p ^= p >> 33;
	p *= UINT64_C(0xc4ceb9fe1a85ec53);
	p ^= p >> 33;

	return p;
}

int equal_pointer(void *p, void *q)
{
	return p == q;
}

/* PJW non-cryptographic string hash function */
size_t hash_str(void *k)
{
	uint32_t h = 0;
	uint32_t high;
	const char *p = k;

	while (*p) {
		h = (h << 4) + *p++;
		if ((high = h & 0xF0000000))
			h ^= high >> 24;
		h &= ~high;
	}

	return h;
}

int equal_str(void *p, void *q)
{
	return strcmp((const char *)p, (const char *)q) == 0;
}

/*
 * Various hash functions
 */

/* multiplier is a prime close to 2^32 x phi */
static uint32_t himult32(void *p)
{
	uintptr_t k = H_POINTER_TO_INT(p);

	return (uint32_t)k * 2654435761;
}

/* for 64 bits, use 11400712997709160919 which is a prime close to 2^64 x phi */
static uint64_t himult64(void *p)
{
	uintptr_t k = H_POINTER_TO_INT(p);

	return k * UINT64_C(11400712997709160919);
}

/* MurmurHash3 based hash functions */
static uint32_t fmix32(void *p)
{
	uintptr_t k = H_POINTER_TO_INT(p);
	uint32_t h = (uint32_t)k;

	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;

	return h;
}

static uint64_t fmix64(void *p)
{
	uint64_t k = H_POINTER_TO_INT(p);

	k ^= k >> 33;
	k *= UINT64_C(0xff51afd7ed558ccd);
	k ^= k >> 33;
	k *= UINT64_C(0xc4ceb9fe1a85ec53);
	k ^= k >> 33;

	return k;
}

/*
  some results:

  doing 89 insertions in a table of length 128 (89 is 128 x 0.7)

  			id  update  method
			++   += 8   random()
  function
  fmix32              28   33     28
  fmix64              30   25     33
  mult_hash            0   42     33
  mult_hash_i          0   72     32
  mult_hash_i64        0   72     28
*/


struct hash_table *hash_table_new(hash_fun_t hash_fun, equal_fun_t equal_fun, destroy_cb_t key_destroy_cb, destroy_cb_t value_destroy_cb)
{
	struct hash_table *ht;

	ht = malloc(sizeof(struct hash_table));
	ht->size = HASH_DEFAULT_SIZE;
	ht->table = calloc(ht->size, sizeof(struct hash_table_entry));
	ht->key_count = 0;

	ht->hash_fun = hash_fun;
	ht->equal_fun = equal_fun;
	ht->key_destroy_cb = key_destroy_cb;
	ht->value_destroy_cb = value_destroy_cb;

	return ht;
}

void hash_table_free(struct hash_table *ht)
{
	size_t i;
	struct hash_table_entry *p;

	for (i = 0, p = ht->table; i < ht->size; i++, p++) {
		if (p->state == EMPTY || p->state == REMOVED)
			continue;

		if (ht->key_destroy_cb != NULL && p->key != NULL)
			(*ht->key_destroy_cb)(p->key);
		if (ht->value_destroy_cb != NULL && p->value != NULL)
			(*ht->value_destroy_cb)(p->value);
	}

	free(ht->table);
	free(ht);
}

void hash_table_print(struct hash_table *ht)
{
	size_t i;
	const char *fmt = (ht->hash_fun == hash_str) ? " key %s value %p\n" : " key %p value %p\n";

	printf("hash table size %ld key count %ld\n", ht->size, ht->key_count);

	for(i = 0; i < ht->size; i++)
		printf(fmt, ht->table[i].key, ht->table[i].value);
}

static void hash_table_rehash(struct hash_table *ht)
{
	size_t old_size;
	size_t j;
	struct hash_table_entry *old_table;

	/* fprintf(stderr, "rehashing: %ld keys vs. %ld size\n", ht->key_count, ht->size); */

	old_size = ht->size;
	old_table = ht->table;
	ht->size = 2 * old_size;
	ht->table = calloc(ht->size, sizeof(struct hash_table_entry));

	for(j = 0; j < old_size; j++) {
		size_t h;
		size_t i;
		size_t w;

		h = HASH(ht, old_table[j].key) % ht->size;

		for (i = 0; i < ht->size; i++) {
			w = (h + i) % ht->size;

			if (IS_EMPTY(ht->table, w))
				break;
		}

		ht->table[w].key = old_table[j].key;
		ht->table[w].value = old_table[j].value;
		ht->table[w].state = OCCUPIED;
	}

	free(old_table);
}

static int hash_table_check_overflow(struct hash_table *ht)
{
	/* 16 / 23 == 0.6956 e.g. ~ 0.7 */
	return 23 * ht->key_count > 16 * ht->size;
}

int hash_table_insert(struct hash_table *ht, void *key, void *value)
{
	size_t h;
	size_t i;
	size_t w;

	if (hash_table_check_overflow(ht))
		hash_table_rehash(ht);

	h = HASH(ht, key) % ht->size;

	for (i = 0; i < ht->size; i++) {
		w = (h + i) % ht->size;

		if (!IS_OCCUPIED(ht->table, w))
			break;
		if (EQUAL(ht, ht->table[w].key, key))
			break;
	}

	/* this should never happen as we check overflow before inserting */
	assert(i != ht->size);

	/* TODO: update collision counter instead of printing */
	/* if (i != 0) */
	/* 	fprintf(stderr, "collision for key %p\n", key); */

	if (ht->key_destroy_cb != NULL && ht->table[w].key != NULL)
		(*ht->key_destroy_cb)(ht->table[w].key);
	ht->table[w].key = key;

	if (ht->value_destroy_cb != NULL && ht->table[w].value != NULL)
		(*ht->value_destroy_cb)(ht->table[w].value);
	ht->table[w].value = value;

	ht->table[w].state = OCCUPIED;

	ht->key_count++;

	return 1;
}

static struct hash_table_entry *lookup_entry(struct hash_table *ht, void *key)
{
	size_t h;
	size_t i;
	size_t w;

	h = HASH(ht, key) % ht->size;

	for (i = 0; i < ht->size; i++) {
		w = (h + i) % ht->size;

		if (IS_EMPTY(ht->table, w))
			return NULL;

		if (IS_REMOVED(ht->table, w))
			continue;

		if (EQUAL(ht, ht->table[w].key, key))
			return ht->table + w;
	}

	return NULL;
}

void *hash_table_search(struct hash_table *ht, void *key)
{
	struct hash_table_entry *p = lookup_entry(ht, key);

	if (p != NULL)
		return p->value;

	return NULL;
}

int hash_table_remove(struct hash_table *ht, void *key)
{
	struct hash_table_entry *p = lookup_entry(ht, key);

	if (p == NULL)
		return 0;

	if (ht->key_destroy_cb != NULL && p->key != NULL)
		(*ht->key_destroy_cb)(p->key);
	p->key = NULL;

	if (ht->value_destroy_cb != NULL && p->value != NULL)
		(*ht->value_destroy_cb)(p->value);
	p->value = NULL;
	p->state = REMOVED;

	ht->key_count--;

	return 1;
}
