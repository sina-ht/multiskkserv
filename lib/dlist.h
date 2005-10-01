/*
 * dlist.h -- doubly linked list data structure
 * (C)Copyright 1998, 99, 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of multiskkserv.
 *
 * Last Modified: Sat Oct  1 10:47:49 2005.
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

typedef void (*Dlist_data_destructor)(void *);
typedef struct _dlist Dlist;
typedef struct _dlist_data {
  void *data;
  Dlist_data_destructor data_destructor;
  Dlist *dl;
  struct _dlist_data *prev;
  struct _dlist_data *next;
} Dlist_data;

typedef int (*Dlist_compfunc)(const void *, const void *);
struct _dlist {
  int size;
  Dlist_compfunc cf;
  Dlist_data *guard;
};

Dlist *dlist_create(void);
Dlist_data *dlist_insert_object(Dlist *, Dlist_data *, void *, Dlist_data_destructor);
Dlist_data *dlist_insert(Dlist *, Dlist_data *, void *);
Dlist_data *dlist_insert_value(Dlist *, Dlist_data *, void *);
Dlist_data *dlist_add_object(Dlist *, void *, Dlist_data_destructor);
Dlist_data *dlist_add(Dlist *, void *);
Dlist_data *dlist_add_value(Dlist *, void *);
Dlist_data *dlist_add_str(Dlist *, char *);
int dlist_delete(Dlist *, Dlist_data *);
int dlist_move_to_top(Dlist *, Dlist_data *);
void dlist_set_compfunc(Dlist *, Dlist_compfunc);
int dlist_sort(Dlist *);
int dlist_destroy(Dlist *);

/* private macros */
#define __dlist_guard(dl) ((dl)->guard)
#define __dlist_dlist(dd) ((dd)->dl)
#define __dlist_data_destructor(dd) ((dd)->data_destructor)

#define dlist_next(dd) ((dd)->next)
#define dlist_prev(dd) ((dd)->prev)
#define dlist_top(dl) dlist_next(__dlist_guard(dl))
#define dlist_head(dl) dlist_prev(__dlist_guard(dl))
#define dlist_size(dl) ((dl)->size)
#define dlist_data(dd) ((dd)->data)
#define dlist_iter(dl,dd) for (dd = dlist_top(dl); dd != __dlist_guard(dl); dd = dlist_next((dd)))

#endif
