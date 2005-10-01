/*
 * hash.c -- Hash Table Library
 * (C)Copyright 1999-2005 by Hiroshi Takekawa
 * This file is part of multiskkserv.
 *
 * Last Modified: Sat Oct  1 10:48:42 2005.
 * $Id$
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <stdlib.h>
#include <signal.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "hash.h"

#define HASH_DELETED_KEY ((Dlist_data *)-1)

static unsigned int default_hash_function(void *, unsigned int);
static unsigned int default_hash_function2(void *, unsigned int);

static Hash hash_template = {
  .hash_function = default_hash_function,
  .hash_function2 = default_hash_function2,
};

static unsigned int
default_hash_function(void *key, unsigned int len)
{
  unsigned char c, *k;
  unsigned int h = 0, l;

  k = key;
  l = len;
  while (len--) {
    c = *k++;
    h += c + (c << 17);
    h ^= (h >> 2);
  }

  return h;
}

static unsigned int
default_hash_function2(void *key, unsigned int len)
{
  unsigned char c, *k;
  unsigned int h = 0, l;

  k = key;
  l = len;
  while (len--) {
    c = *k++;
    h += c + (c << 13);
    h ^= (h >> 3);
  }

  return 17 - (h % 17);
}

Hash *
hash_create(int size)
{
  int i;
  Hash *h;
  Hash_data *d;

  h = (Hash *)calloc(1, sizeof(Hash));
  if (unlikely(h == NULL))
    return NULL;
  memcpy(h, &hash_template, sizeof(Hash));

  /* 16411, 8209, 4099, 1031, 257 */
  if (size == 16384 || size == 8192 || size == 4096 || size == 1024 || size == 256)
    warning_fnc("hash_size[%d] must be prime!\n", size);

  h->data = (Hash_data **)calloc(size, sizeof(Hash_data *));
  if (unlikely(h->data == NULL))
    goto error_h;

  d = (Hash_data *)calloc(size, sizeof(Hash_data));
  if (unlikely(d == NULL))
    goto error_data;

  for (i = 0; i < size; i++)
    h->data[i] = d++;

  h->keys = dlist_create();
  if (unlikely(h->keys == NULL))
    goto error_d;

  h->size = size;

  return h;

 error_d:    free(d);
 error_data: free(h->data);
 error_h:    free(h);
  return NULL;
}

static Hash_data *
lookup_key(Hash *h, void *k, unsigned int len)
{
  int count = 0;
  unsigned int hash, first_hash, skip;
  Hash_data *d;
  Hash_key *hk;
  Dlist_data *dd;

  hash = h->hash_function(k, len) % h->size;
  first_hash = hash;
  skip = h->hash_function2(k, len);
  for (;;) {
    d = h->data[hash];
    dd = d->key;

    if (dd == NULL)
      return NULL;

    if (dd != HASH_DELETED_KEY) {
      hk = dlist_data(dd);
      if (hk->len == len && memcmp(hk->key, k, len) == 0)
	break;
    }

    hash = (hash + skip) % h->size;
    if (hash == first_hash) {
      debug_message_fnc("*** hash wrapped around. ***\n");
      return NULL;
    }

    count++;
    bug_on(count > 1030);
  }

  return d;
}

static unsigned int
lookup_key_or_vacancy(Hash *h, void *k, unsigned int len)
{
  int count = 0;
  unsigned int hash, skip;
  Hash_key *hk;
  Dlist_data *dd;

  hash = h->hash_function(k, len) % h->size;
  skip = h->hash_function2(k, len);
  for (;;) {
    dd = h->data[hash]->key;
    if (dd == HASH_DELETED_KEY || dd == NULL)
      break;
    hk = dlist_data(dd);
    if (hk->len == len && memcmp(hk->key, k, len) == 0)
      break;
    hash = (hash + skip) % h->size;

    count++;
    bug_on(count > 1000);
  }

  return hash;
}

static Hash_key *
hash_key_create(void *k, unsigned int len)
{
  Hash_key *hk;

  hk = malloc(sizeof(Hash_key));
  if (unlikely(hk == NULL))
    return NULL;

  hk->key = malloc(len);
  if (unlikely(hk->key == NULL)) {
    free(hk);
    return NULL;
  }

  memcpy(hk->key, k, len);
  hk->len = len;

  return hk;
}

static void
hash_key_destroy(void *arg)
{
  Hash_key *hk = arg;

  if (hk) {
    if (hk->key)
      free(hk->key);
    free(hk);
  }
}

/* methods */

void
hash_set_function(Hash *h, unsigned int (*function)(void *, unsigned int))
{
  h->hash_function = function;
}

void
hash_set_function2(Hash *h, unsigned int (*function2)(void *, unsigned int))
{
  h->hash_function2 = function2;
}

int
hash_define_object(Hash *h, void *k, unsigned int len, void *d, Hash_data_destructor dest)
{
  unsigned int i = lookup_key_or_vacancy(h, k, len);
  Hash_key *hk;

  if (h->data[i]->key != NULL && h->data[i]->key != HASH_DELETED_KEY)
    return -1; /* already registered */

  hk = hash_key_create(k, len);
  if (unlikely(hk == NULL))
    return 0;

  h->data[i]->key = dlist_add_object(h->keys, hk, hash_key_destroy);
  if (unlikely(h->data[i]->key == NULL)) {
    hash_key_destroy(hk);
    return 0;
  }

  h->data[i]->datum = d;
  h->data[i]->data_destructor = dest;

  return 1;
}

int
hash_define(Hash *h, void *k, unsigned int len, void *d)
{
  return hash_define_object(h, k, len, d, free);
}

int
hash_define_value(Hash *h, void *k, unsigned int len, void *d)
{
  return hash_define_object(h, k, len, d, NULL);
}

static void
destroy_hash_data(Hash_data *d)
{
  if (d->datum && d->data_destructor)
    d->data_destructor(d->datum);
}

int
hash_set_object(Hash *h, void *k, unsigned int len, void *d, Hash_data_destructor dest)
{
  unsigned int i = lookup_key_or_vacancy(h, k, len);
  Hash_key *hk;

  if (h->data[i]->key == NULL || h->data[i]->key == HASH_DELETED_KEY) {
    hk = hash_key_create(k, len);
    if (unlikely(hk == NULL))
      return 0;
    h->data[i]->key = dlist_add_object(h->keys, hk, hash_key_destroy);
    if (unlikely(h->data[i]->key == NULL)) {
      hash_key_destroy(hk);
      return 0;
    }
  } else {
    /* Should destroy overwritten data */
    destroy_hash_data(h->data[i]);
  }

  h->data[i]->datum = d;
  h->data[i]->data_destructor = dest;

  return 1;
}

int
hash_set(Hash *h, void *k, unsigned int len, void *d)
{
  return hash_set_object(h, k, len, d, free);
}

int
hash_set_value(Hash *h, void *k, unsigned int len, void *d)
{
  return hash_set_object(h, k, len, d, NULL);
}

int
hash_get_key_size(Hash *h)
{
  Dlist *keys = hash_get_keys(h);
  return dlist_size(keys);
}

void *
hash_lookup(Hash *h, void *k, unsigned int len)
{
  Hash_data *d = lookup_key(h, k, len);

  if (d == NULL)
    return NULL;
  return d->datum;
}

static int
delete_key(Hash *h, Hash_data *d)
{
  if (d->key == HASH_DELETED_KEY)
    return 0;
  if (d->key != NULL) {
    if (unlikely(!dlist_delete(h->keys, d->key)))
      return 0;
    d->key = HASH_DELETED_KEY;
  }
  destroy_hash_data(d);

  return 1;
}

int
hash_delete(Hash *h, void *k, unsigned int len)
{
  Hash_data *d = lookup_key(h, k, len);

  if (d == NULL)
    return 0;

  if (unlikely(!delete_key(h, d)))
    return 0;

  return 1;
}

void
hash_destroy(Hash *h)
{
  Dlist_data *t;
  Dlist *keys = hash_get_keys(h);
  Hash_key *hk;

  while (hash_get_key_size(h) > 0) {
    t = dlist_top(keys);
    hk = dlist_data(t);
    if (unlikely(!hash_delete(h, hk->key, hk->len))) {
      err_message_fnc("size = %d\n", hash_get_key_size(h));
      break;
    }
  }

  dlist_destroy(keys);
  free(h->data[0]);
  free(h->data);
  free(h);
}
