#include "modules/collections/hash_table.h"

#include <stdlib.h>

// Derived from code by Syoyo Fujita and other many contributors at https://github.com/syoyo/tinyobjloader-c

unsigned long hash_djb2(const unsigned char *str)
{
  unsigned long hash = 5381;
  int c;

  while ((c = *str++)) {
    hash = ((hash << 5) + hash) + (unsigned long)(c);
  }

  return hash;
}

void create_hash_table(size_t start_capacity, hash_table_t *hash_table)
{
  if (start_capacity < 1)
    start_capacity = HASH_TABLE_DEFAULT_SIZE;
  hash_table->hashes = (unsigned long *)malloc(start_capacity * sizeof(unsigned long));
  hash_table->entries = (hash_table_entry_t *)calloc(start_capacity, sizeof(hash_table_entry_t));
  hash_table->capacity = start_capacity;
  hash_table->n = 0;
}

void destroy_hash_table(hash_table_t *hash_table)
{
  free(hash_table->entries);
  free(hash_table->hashes);
}

/* Insert with quadratic probing */
int hash_table_insert_value(unsigned long hash, long value, hash_table_t *hash_table)
{
  /* Insert value */
  size_t start_index = hash % hash_table->capacity;
  size_t index = start_index;
  hash_table_entry_t *start_entry = hash_table->entries + start_index;
  size_t i;
  hash_table_entry_t *entry;

  for (i = 1; hash_table->entries[index].filled; i++) {
    if (i >= hash_table->capacity)
      return HASH_TABLE_ERROR;
    index = (start_index + (i * i)) % hash_table->capacity;
  }

  entry = hash_table->entries + index;
  entry->hash = hash;
  entry->filled = 1;
  entry->value = value;

  if (index != start_index) {
    /* This is a new entry, but not the start entry, hence we need to add a next pointer to our entry */
    entry->next = start_entry->next;
    start_entry->next = entry;
  }

  return HASH_TABLE_SUCCESS;
}

int hash_table_insert(unsigned long hash, long value, hash_table_t *hash_table)
{
  int ret = hash_table_insert_value(hash, value, hash_table);
  if (ret == HASH_TABLE_SUCCESS) {
    hash_table->hashes[hash_table->n] = hash;
    hash_table->n++;
  }
  return ret;
}

hash_table_entry_t *hash_table_find(unsigned long hash, hash_table_t *hash_table)
{
  hash_table_entry_t *entry = hash_table->entries + (hash % hash_table->capacity);
  while (entry) {
    if (entry->hash == hash && entry->filled) {
      return entry;
    }
    entry = entry->next;
  }
  return NULL;
}

void hash_table_maybe_grow(size_t new_n, hash_table_t *hash_table)
{
  size_t new_capacity;
  hash_table_t new_hash_table;
  size_t i;

  if (new_n <= hash_table->capacity) {
    return;
  }
  new_capacity = 2 * ((2 * hash_table->capacity) > new_n ? hash_table->capacity : new_n);
  /* Create a new hash table. We're not calling create_hash_table because we want to realloc the hash array */
  new_hash_table.hashes = hash_table->hashes =
      (unsigned long *)realloc((void *)hash_table->hashes, sizeof(unsigned long) * new_capacity);
  new_hash_table.entries = (hash_table_entry_t *)calloc(new_capacity, sizeof(hash_table_entry_t));
  new_hash_table.capacity = new_capacity;
  new_hash_table.n = hash_table->n;

  /* Rehash */
  for (i = 0; i < hash_table->capacity; i++) {
    hash_table_entry_t *entry = hash_table_find(hash_table->hashes[i], hash_table);
    hash_table_insert_value(hash_table->hashes[i], entry->value, &new_hash_table);
  }

  free(hash_table->entries);
  (*hash_table) = new_hash_table;
}

int hash_table_exists(const char *name, hash_table_t *hash_table)
{
  unsigned long hash = hash_djb2((const unsigned char *)name);
  hash_table_entry_t *res = hash_table_find(hash, hash_table);
  return res != NULL;
}

void hash_table_set(const char *name, size_t val, hash_table_t *hash_table)
{
  /* Hash name */
  unsigned long hash = hash_djb2((const unsigned char *)name);

  hash_table_entry_t *entry = hash_table_find(hash, hash_table);
  if (entry) {
    entry->value = (long)val;
    return;
  }

  /* Expand if necessary
   * Grow until the element has been added
   */
  long ht_insert;
  do {
    hash_table_maybe_grow(hash_table->n + 1, hash_table);
    ht_insert = hash_table_insert(hash, (long)val, hash_table);
  } while (ht_insert != HASH_TABLE_SUCCESS);
}

long hash_table_get(const char *name, hash_table_t *hash_table)
{
  unsigned long hash = hash_djb2((const unsigned char *)name);
  hash_table_entry_t *ret = hash_table_find(hash, hash_table);
  return ret->value;
}