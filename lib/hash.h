/*
 * hash.h -- Hash Table Library Header
 * (C)Copyright 1999, 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of multiskkserv.
 *
 * Last Modified: Fri Feb  1 01:45:59 2002.
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

#ifndef _HASH_H
#define _HASH_H

#include "dlist.h"

#define HASH_LOOKUP_SEARCH_MATCH 0
#define HASH_LOOKUP_ACCEPT_DELETED 1

typedef struct {
  void *key;
  unsigned int len;
} Hash_key;

typedef struct {
  Dlist_data *key;
  void *datum;
} Hash_data;

typedef struct _hash Hash;
struct _hash {
  unsigned int size;
  Hash_data **data;
  Dlist *keys;
  unsigned int (*hash_function)(void *, unsigned int);
  unsigned int (*hash_function2)(void *, unsigned int);

  int (*define)(Hash *, void *, unsigned int, void *);
  int (*set)(Hash *, void *, unsigned int, void *);
  void (*set_function)(Hash *, unsigned int (*)(void *, unsigned int));
  void (*set_function2)(Hash *, unsigned int (*)(void *, unsigned int));
  Dlist *(*get_keys)(Hash *);
  int (*get_key_size)(Hash *);
  void *(*lookup)(Hash *, void *, unsigned int);
  int (*delete_key)(Hash *, void *, unsigned int, int);
  void (*destroy)(Hash *, int);
};

#define hash_define(h, k, l, d) (h)->define((h), (k), (l), (d))
#define hash_set(h, k, l, d) (h)->set((h), (k), (l), (d))
#define hash_lookup(h, k, l) (h)->lookup((h), (k), (l))
#define hash_delete(h, k, l, f) (h)->delete_key((h), (k), (l), (f))

/* avoid side effect */
#define hash_define_str(h, k, d) ({void * _k = (void *)(k); (h)->define((h), _k, strlen((const char *)_k) + 1, (d));})
#define hash_set_str(h, k, d) ({void * _k = (void *)(k); (h)->set((h), _k, strlen((const char *)_k) + 1, (d));})
#define hash_lookup_str(h, k) ({void * _k = (void *)(k); (h)->lookup((h), _k, strlen((const char *)_k) + 1);})
#define hash_delete_str(h, k, f) ({void * _k = (void *)(k); (h)->delete_key((h), _k, strlen((const char *)_k) + 1, (f));})

#define hash_set_function(h, func) (h)->set_function((h), (func))
#define hash_set_function2(h, func) (h)->set_function2((h), (func))
#define hash_get_keys(h) (h)->get_keys((h))
#define hash_get_key_size(h) (h)->get_key_size((h))
#define hash_destroy(h, f) (h)->destroy((h), (f))

#define hash_key_key(hk) ((Hash_key *)hk)->key
#define hash_key_len(hk) ((Hash_key *)hk)->len

#define hash_iter(h, dl, dd, hk, d) \
 dl = hash_get_keys(h); \
 dd = dlist_top((dl)); \
 hk = dlist_data((dd)); \
 if (hk) \
   for (d = hash_lookup((h), (hk->key), (hk->len)); \
        dd != NULL && (hk = dlist_data((dd))) && (d = hash_lookup((h), (hk->key), (hk->len))); \
        dd = dlist_next((dd)))

Hash *hash_create(int);

#endif
