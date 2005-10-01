/*
 * hash.h -- Hash Table Library Header
 * (C)Copyright 1999, 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of multiskkserv.
 *
 * Last Modified: Sat Oct  1 10:48:54 2005.
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

typedef struct {
  void *key;
  unsigned int len;
} Hash_key;

typedef void (*Hash_data_destructor)(void *);
typedef struct {
  Dlist_data *key;
  void *datum;
  Hash_data_destructor data_destructor;
} Hash_data;

typedef struct _hash Hash;
struct _hash {
  unsigned int size;
  Hash_data **data;
  Dlist *keys;
  unsigned int (*hash_function)(void *, unsigned int);
  unsigned int (*hash_function2)(void *, unsigned int);
};

Hash *hash_create(int);
int hash_define_object(Hash *, void *, unsigned int, void *, Hash_data_destructor);
int hash_define(Hash *, void *, unsigned int, void *);
int hash_define_value(Hash *, void *, unsigned int, void *);
int hash_set_object(Hash *, void *, unsigned int, void *, Hash_data_destructor);
int hash_set(Hash *, void *, unsigned int, void *);
int hash_set_value(Hash *, void *, unsigned int, void *);
void *hash_lookup(Hash *, void *, unsigned int);
int hash_delete(Hash *, void *, unsigned int);
void hash_set_function(Hash *, unsigned int (*)(void *, unsigned int));
void hash_set_function2(Hash *, unsigned int (*)(void *, unsigned int));
int hash_get_key_size(Hash *);
void hash_destroy(Hash *);

/* avoid side effect */
#define hash_define_str(h, k, d) ({void * _k = (void *)(k); hash_define((h), _k, strlen((const char *)_k) + 1, (d));})
#define hash_set_str(h, k, d) ({void * _k = (void *)(k); hash_set((h), _k, strlen((const char *)_k) + 1, (d));})
#define hash_define_str_value(h, k, d) ({void * _k = (void *)(k); hash_define_value((h), _k, strlen((const char *)_k) + 1, (d));})
#define hash_set_str_value(h, k, d) ({void * _k = (void *)(k); hash_set_value((h), _k, strlen((const char *)_k) + 1, (d));})
#define hash_lookup_str(h, k) ({void * _k = (void *)(k); hash_lookup((h), _k, strlen((const char *)_k) + 1);})
#define hash_delete_str(h, k) ({void * _k = (void *)(k); hash_delete((h), _k, strlen((const char *)_k) + 1);})

#define hash_get_keys(h) ((h)->keys)

#define hash_key_key(hk) ((Hash_key *)hk)->key
#define hash_key_len(hk) ((Hash_key *)hk)->len

#define hash_iter(h, k, kl, d) { \
 Dlist *__dl = hash_get_keys(h); \
 Dlist_data *__dd = dlist_top(__dl); \
 Hash_key *__hk = dlist_data(__dd); \
 if (__hk) \
   for (k = __hk->key, kl = __hk->len, d = hash_lookup((h), k, kl); \
        __dd != NULL && (__hk = dlist_data(__dd)) && (k = __hk->key, kl = __hk->len, d = hash_lookup((h), k, kl)); \
        __dd = dlist_next(__dd))
#define hash_iter_dl __dl
#define hash_iter_dd __dd
#define hash_iter_end }

#endif
