/*
 * dlist.h -- doubly linked list data structure
 * (C)Copyright 1998, 99, 2000, 2001 by Hiroshi Takekawa
 * This file is part of multiskkserv.
 *
 * Last Modified: Mon Feb 12 00:20:14 2001.
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

#ifndef _DLIST_H_
#define _DLIST_H_

typedef struct _dlist Dlist;
typedef struct _dlist_data {
  void *data;
  Dlist *dl;
  struct _dlist_data *prev;
  struct _dlist_data *next;
} Dlist_data;

typedef int (*Dlist_compfunc)(const void *, const void *);

struct _dlist {
  Dlist_compfunc cf;
  int size;
  Dlist_data *guard;

  int (*attach)(Dlist *, Dlist_data *, Dlist_data *);
  Dlist_data *(*insert)(Dlist *, Dlist_data *, void *);
  Dlist_data *(*add)(Dlist *, void *);
  Dlist_data *(*add_str)(Dlist *, char *);
  int (*detach)(Dlist *, Dlist_data *);
  int (*delete_item)(Dlist *, Dlist_data *);
  int (*move_to_top)(Dlist *, Dlist_data *);
  void (*set_compfunc)(Dlist *, Dlist_compfunc);
  int (*do_sort)(Dlist *);
  int (*destroy)(Dlist *, int);
};

#define dlist_attach(dl, d1, d2) (dl)->attach((dl), (d1), (d2))
#define dlist_insert(dl, dd, d) (dl)->insert((dl), (dd), (d))
#define dlist_add(dl, d) (dl)->add((dl), (d))
#define dlist_add_str(dl, d) (dl)->add_str((dl), (d))
#define dlist_detach(dl, dd) (dl)->detach((dl), (dd))
#define dlist_delete(dl, dd) (dl)->delete_item((dl), (dd))
#define dlist_move_to_top(dl, dd) (dl)->move_to_top((dl), (dd))
#define dlist_set_compfunc(dl, cf) (dl)->set_compfunc((dl), (cf))
#define dlist_sort(dl) (dl)->do_sort((dl))
#define dlist_destroy(dl, f) (dl)->destroy((dl), (f))

#define dlist_guard(dl) ((dl)->guard)
#define dlist_top(dl) (dlist_next(dlist_guard(dl)))
#define dlist_head(dl) (dlist_prev(dlist_guard(dl)))
#define dlist_size(dl) ((dl)->size)

#define dlist_dlist(dd) ((dd)->dl)
#define dlist_next(dd) ((dd)->next)
#define dlist_prev(dd) ((dd)->prev)
#define dlist_data(dd) ((dd)->data)

#define dlist_iter(dl,dd) for (dd = dlist_top(dl); dd != dlist_guard(dl); dd = dlist_next((dd)))

Dlist *dlist_create(void);

#endif
