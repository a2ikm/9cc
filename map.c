#include "9cc.h"

#define INITIAL_SIZE 16
#define BORDER_RATE 70
#define TOMBSTONE ((void *)-1)

// FNV-1a
// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
static uint64_t fnv_hash(char *s, int len)
{
  uint64_t hash = 0xcbf29ce484222325;
  for (int i = 0; i < len; i++)
  {
    hash ^= (unsigned char)s[i];
    hash *= 0x100000001b3;
  }
  return hash;
}

static void rehash(Map *map)
{
  if (map->len * 100 < map->size * BORDER_RATE)
  {
    return;
  }

  int len = 0;
  for (int i = 0; i < map->size; i++)
  {
    MapEntry *entry = &map->buckets[i];
    if (entry && entry->key != TOMBSTONE)
    {
      len++;
    }
  }

  int size = map->size;
  while (size * BORDER_RATE <= len * 100)
  {
    size *= 2;
  }

  Map *map2 = malloc(sizeof(Map));
  map2->size = size;
  map2->buckets = calloc(size, sizeof(MapEntry));

  for (int i = 0; i < map->size; i++)
  {
    MapEntry *entry = &map->buckets[i];
    if (entry && entry->key && entry->key != TOMBSTONE)
    {
      map_put2(map2, entry->key, entry->keylen, entry->data);
    }
  }

  *map = *map2;
}

static bool match(MapEntry *entry, char *key, int keylen)
{
  return entry && entry->key != TOMBSTONE &&
         entry->keylen == keylen && memcmp(entry->key, key, keylen) == 0;
}

static MapEntry *get_entry(Map *map, char *key, int keylen)
{
  if (map->len == 0)
  {
    return NULL;
  }

  uint64_t hash = fnv_hash(key, keylen);

  for (int i = 0; i < map->size; i++)
  {
    MapEntry *entry = &map->buckets[(hash + i) % map->size];
    if (match(entry, key, keylen))
    {
      return entry;
    }
    if (entry->key == NULL)
    {
      return NULL;
    }
  }

  unreachable();
}

static MapEntry *get_or_insert_entry(Map *map, char *key, int keylen)
{
  rehash(map);

  uint64_t hash = fnv_hash(key, keylen);

  for (int i = 0; i < map->size; i++)
  {
    MapEntry *entry = &map->buckets[(hash + i) % map->size];
    if (match(entry, key, keylen))
    {
      return entry;
    }
    if (entry->key == TOMBSTONE)
    {
      entry->key = key;
      entry->keylen = keylen;
      return entry;
    }
    if (entry->key == NULL)
    {
      entry->key = key;
      entry->keylen = keylen;
      map->len++;
      return entry;
    }
  }

  unreachable();
}

Map *map_new()
{
  Map *map = malloc(sizeof(Map));
  map->len = 0;
  map->size = INITIAL_SIZE;
  map->buckets = malloc(sizeof(MapEntry) * map->size);
  return map;
}

void *map_get(Map *map, char *key)
{
  return map_get2(map, key, strlen(key));
}

void *map_get2(Map *map, char *key, int keylen)
{
  MapEntry *entry = get_entry(map, key, keylen);
  if (entry)
  {
    return entry->data;
  }
  else
  {
    return NULL;
  }
}

void map_put(Map *map, char *key, void *data)
{
  map_put2(map, key, strlen(key), data);
}

void map_put2(Map *map, char *key, int keylen, void *data)
{
  MapEntry *entry = get_or_insert_entry(map, key, keylen);
  entry->data = data;
}

void map_delete(Map *map, char *key)
{
  map_delete2(map, key, strlen(key));
}

void map_delete2(Map *map, char *key, int keylen)
{
  MapEntry *entry = get_entry(map, key, keylen);
  if (entry)
  {
    entry->key = TOMBSTONE;
  }
}

Vector *map_keys(Map *map)
{
  Vector *keys = vec_new();

  for (int i = 0; i < map->size; i++)
  {
    MapEntry *entry = &map->buckets[i];
    if (entry && entry->key && entry->key != TOMBSTONE)
    {
      vec_add(keys, strndup(entry->key, entry->keylen));
    }
  }

  return keys;
}

void map_test()
{
  Map *map = map_new();

  for (int i = 0; i < INITIAL_SIZE + 1; i++)
  {
    int *ret;
    Vector *keys;

    int len = snprintf(NULL, 0, "key%d", i);
    char *key = malloc(sizeof(char) * len);
    sprintf(key, "key%d", i);

    int *obj = &i;

    ret = (int *)map_get(map, key);
    assert(ret == NULL);

    keys = map_keys(map);
    assert(vec_len(keys) == 0);

    map_put(map, key, obj);

    ret = (int *)map_get(map, key);
    assert(ret == obj);

    keys = map_keys(map);
    assert(vec_len(keys) == 1);
    assert(memcmp(vec_get(keys, 0), key, len) == 0);

    map_delete(map, key);

    ret = (int *)map_get(map, key);
    assert(ret == NULL);

    keys = map_keys(map);
    assert(vec_len(keys) == 0);
  }
}
