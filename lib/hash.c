/*
 * hash.c -- Hash Table Library
 * (C)Copyright 1999, 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of multiskkserv.
 *
 * Last Modified: Fri Feb  1 01:45:40 2002.
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

static int define(Hash *, void *, unsigned int, void *);
static int set(Hash *, void *, unsigned int, void *);
static void set_function(Hash *, unsigned int (*)(void *, unsigned int));
static void set_function2(Hash *, unsigned int (*)(void *, unsigned int));
static Dlist *get_keys(Hash *);
static int get_key_size(Hash *);
static void *lookup(Hash *, void *, unsigned int);
static int delete_key(Hash *, void *, unsigned int, int);
static void destroy(Hash *, int);

static unsigned int default_hash_function(void *, unsigned int);
static unsigned int default_hash_function2(void *, unsigned int);

static Hash hash_template = {
  hash_function: default_hash_function,
  hash_function2: default_hash_function2,

  define: define,
  set: set,
  set_function: set_function,
  set_function2: set_function2,
  get_keys: get_keys,
  get_key_size: get_key_size,
  lookup: lookup,
  delete_key: delete_key,
  destroy: destroy
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
  h += l + (l << 17);
  h ^= (h >> 2);

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
  h += l + (l << 13);
  h ^= (h >> 3);

  return 17 - (h % 17);
}

Hash *
hash_create(int size)
{
  int i;
  Hash *h;
  Hash_data *d;

  if ((h = (Hash *)calloc(1, sizeof(Hash))) == NULL)
    return NULL;

  memcpy(h, &hash_template, sizeof(Hash));

  if ((h->data = (Hash_data **)calloc(size, sizeof(Hash_data *))) == NULL)
    goto error_h;

  if ((d = (Hash_data *)calloc(size, sizeof(Hash_data))) == NULL)
    goto error_data;

  for (i = 0; i < size; i++)
    h->data[i] = d++;

  if ((h->keys = dlist_create()) == NULL)
    goto error_d;

  h->size = size;

  return h;

 error_d:    free(d);
 error_data: free(h->data);
 error_h:    free(h);
  return NULL;
}

static unsigned int
lookup_internal(Hash *h, void *k, unsigned int len, int flag)
{
  int count = 0;
  unsigned int hash, skip;
  unsigned char *kc;
  Hash_key *hk;

  kc = k;
  hash = h->hash_function(k, len) % h->size;
  skip = h->hash_function2(k, len);

  do {
    if (h->data[hash]->key == (Dlist_data *)-1) {
      if (flag == HASH_LOOKUP_ACCEPT_DELETED)
	break;
      hash = (hash + skip) % h->size;
    } else if (!h->data[hash]->key) {
      /* found empty */
      break;
    } else if ((hk = dlist_data(h->data[hash]->key)) &&
	       hk->len == len && (memcmp(hk->key, k, len) == 0)) {
      break;
    } else {
      hash = (hash + skip) % h->size;
    }
    count++;
    if (count > 100000)
      raise(SIGABRT);
  } while (1);

  return hash;
}

static Hash_key *
hash_key_create(void *k, unsigned int len)
{
  Hash_key *hk;

  if ((hk = malloc(sizeof(Hash_key))) == NULL)
    return NULL;
  if ((hk->key = malloc(len)) == NULL) {
    free(hk);
    return NULL;
  }
  memcpy(hk->key, k, len);
  hk->len = len;

  return hk;
}

/* methods */

static void
set_function(Hash *h, unsigned int (*function)(void *, unsigned int))
{
  h->hash_function = function;
}

static void
set_function2(Hash *h, unsigned int (*function2)(void *, unsigned int))
{
  h->hash_function2 = function2;
}

static int
define(Hash *h, void *k, unsigned int len, void *d)
{
  unsigned int i;
  Hash_key *hk;

  i = lookup_internal(h, k, len, HASH_LOOKUP_ACCEPT_DELETED);
  if (h->data[i]->key != NULL && h->data[i]->key != (Dlist_data *)-1)
    return -1; /* already registered */

  if ((hk = hash_key_create(k, len)) == NULL)
    return 0;
  if ((h->data[i]->key = h->keys->add(h->keys, hk)) == NULL) {
    free(hk);
    return 0;
  }

  h->data[i]->datum = d;

  return 1;
}

static int
set(Hash *h, void *k, unsigned int len, void *d)
{
  unsigned int i;
  Hash_key *hk;

  i = lookup_internal(h, k, len, HASH_LOOKUP_ACCEPT_DELETED);
  if (h->data[i]->key == NULL || h->data[i]->key == (Dlist_data *)-1) {
    if ((hk = hash_key_create(k, len)) == NULL)
      return 0;
    if ((h->data[i]->key = h->keys->add(h->keys, hk)) == NULL) {
      free(hk);
      return 0;
    }
  }

  h->data[i]->datum = d;

  return 1;
}

static Dlist *
get_keys(Hash *h)
{
  return h->keys;
}

static int
get_key_size(Hash *h)
{
  Dlist *keys;

  keys = get_keys(h);
  return dlist_size(keys);
}

static void *
lookup(Hash *h, void *k, unsigned int len)
{
  unsigned int i;

  i = lookup_internal(h, k, len, HASH_LOOKUP_SEARCH_MATCH);

  return (h->data[i]->key == NULL) ? NULL : h->data[i]->datum;
}

static int
destroy_datum(Hash *h, int i, int f)
{
  Hash_data *d = h->data[i];

  if (d->key == (Dlist_data *)-1)
    return 0;
  if (d->key != NULL) {
    if (!dlist_delete(h->keys, d->key))
      return 0;
    d->key = (Dlist_data *)-1;
  }
  if (f && d->datum != NULL)
    free(d->datum);

  return 1;
}

static int
delete_key(Hash *h, void *k, unsigned int len, int f)
{
  int i = lookup_internal(h, k, len, HASH_LOOKUP_SEARCH_MATCH);

  if (h->data[i]->key == NULL)
    return 0;

  if (!destroy_datum(h, i, f))
    return 0;

  return 1;
}

static void
destroy(Hash *h, int f)
{
  Dlist_data *t;
  Dlist *keys;
  Hash_key *hk;

  keys = get_keys(h);
  while (get_key_size(h) > 0) {
    t = dlist_top(keys);
    hk = dlist_data(t);
    delete_key(h, hk->key, hk->len, f);
  }

  dlist_destroy(keys, 1);
  free(h->data[0]);
  free(h->data);
  free(h);
}
