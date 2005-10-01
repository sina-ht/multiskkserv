/*
 * dlist.c -- double-linked list data structure
 * (C)Copyright 1998, 99, 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of multiskkserv.
 *
 * Last Modified: Sat Oct  1 10:47:29 2005.
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
#include "dlist.h"

static Dlist_data *
dlist_data_create(Dlist *dl)
{
  Dlist_data *dd = calloc(1, sizeof(Dlist_data));

  if (unlikely(dd == NULL))
    return NULL;
  dd->dl = dl;

  return dd;
}

Dlist *
dlist_create(void)
{
  Dlist *dl = calloc(1, sizeof(Dlist));

  if (unlikely(dl == NULL))
    goto error;

  dl->guard = dlist_data_create(dl);
  if (unlikely(dl->guard  == NULL))
    goto error;
  dlist_prev(dl->guard) = dl->guard;
  dlist_next(dl->guard) = dl->guard;

  return dl;

 error:
  if (dl)
    free(dl);

  return NULL;
}

static void
destroy_dlist_data(Dlist_data *dd)
{
  void *ddd = dlist_data(dd);
  Dlist_data_destructor dddest = __dlist_data_destructor(dd);

  if (ddd && dddest)
    dddest(ddd);
}

int
dlist_destroy(Dlist *dl)
{
  Dlist_data *dd, *dd_n, *g;

  g = __dlist_guard(dl);
  dlist_next(dlist_prev(g)) = NULL;
  dd_n = dlist_next(g);
  free(g);
  for (dd = dd_n; dd; dd = dd_n) {
    dd_n = dlist_next(dd);
    destroy_dlist_data(dd);
    free(dd);
  }
  free(dl);

  return 1;
}

static inline int
dlist_attach(Dlist *dl, Dlist_data *inserted, Dlist_data *inserted_n)
{
  Dlist_data *inserted_p = dlist_prev(inserted_n);

  dlist_next(inserted_p) = inserted;
  dlist_prev(inserted) = inserted_p;
  dlist_next(inserted) = inserted_n;
  dlist_prev(inserted_n) = inserted;

  dlist_size(dl)++;

  return 1;
}

Dlist_data *
dlist_insert_object(Dlist *dl, Dlist_data *inserted, void *d, Dlist_data_destructor dddest)
{
  Dlist_data *dd;

#ifdef DEBUG
  bug_on(__dlist_dlist(inserted) != dl);
#endif

  dd = dlist_data_create(dl);
  if (unlikely(dd == NULL))
    return NULL;
  dlist_data(dd) = d;
  __dlist_data_destructor(dd) = dddest;

  dlist_attach(dl, dd, inserted);

  return dd;
}

Dlist_data *
dlist_insert(Dlist *dl, Dlist_data *inserted, void *d)
{
  return dlist_insert_object(dl, inserted, d, free);
}

Dlist_data *
dlist_insert_value(Dlist *dl, Dlist_data *inserted, void *d)
{
  return dlist_insert_object(dl, inserted, d, NULL);
}

Dlist_data *
dlist_add_object(Dlist *dl, void *d, Dlist_data_destructor dddest)
{
  return dlist_insert_object(dl, __dlist_guard(dl), d, dddest);
}

Dlist_data *
dlist_add(Dlist *dl, void *d)
{
  return dlist_insert(dl, __dlist_guard(dl), d);
}

Dlist_data *
dlist_add_value(Dlist *dl, void *d)
{
  return dlist_insert_value(dl, __dlist_guard(dl), d);
}

Dlist_data *
dlist_add_str(Dlist *dl, char *str)
{
  char *new_str;

  if (str == NULL)
    return NULL;
  new_str = strdup(str);
  if (unlikely(new_str == NULL))
    return NULL;

  return dlist_add(dl, (void *)new_str);
}

static inline int
dlist_detach(Dlist *dl, Dlist_data *dd)
{
  Dlist_data *dd_p, *dd_n;

  if (__dlist_guard(dl) == dd)
    return 0;

  dd_p = dlist_prev(dd);
  dd_n = dlist_next(dd);
  dlist_next(dd_p) = dd_n;
  dlist_prev(dd_n) = dd_p;

  dlist_size(dl)--;

  return 1;
}

int
dlist_delete(Dlist *dl, Dlist_data *dd)
{
  if (dl == NULL || dd == NULL)
    return 0;
#ifdef DEBUG
  bug_on(__dlist_dlist(dd) != dl);
#endif

  if (!dlist_detach(dl, dd))
    return 0;

  destroy_dlist_data(dd);
  free(dd);

  return 1;
}

int
dlist_move_to_top(Dlist *dl, Dlist_data *dd)
{
#ifdef DEBUG
  bug_on(__dlist_dlist(dd) != dl);
#endif
  if (dlist_top(dl) == dd)
    return 1;

  if (!dlist_detach(dl, dd))
    return 0;

  return dlist_attach(dl, dd, dlist_top(dl));
}

void
dlist_set_compfunc(Dlist *dl, Dlist_compfunc cf)
{
  dl->cf = cf;
}

#if 0
static int
validate(Dlist *dl)
{
  Dlist_data *i;

  for (i = get_top(dl); i && i != get_head(dl); i = i->next);
  if (i != get_head(dl))
    return 0;
  for (i = get_head(dl); i && i != get_top(dl); i = i->prev);
  if (i != get_top(dl))
    return 0;
  return 1;
}
#endif

int
dlist_sort(Dlist *dl)
{
  int i;
  Dlist_data *t;
  void **tmp;

  if (dlist_size(dl) <= 1)
    return 1;

  tmp = calloc(dlist_size(dl), sizeof(void *));
  if (unlikely(tmp == NULL))
    return 0;
  t = dlist_top(dl);
  for (i = 0; i < dlist_size(dl); i++) {
    tmp[i] = t->data;
    t = t->next;
  }
  qsort(&tmp[0], dlist_size(dl), sizeof(void *), dl->cf);
  t = dlist_top(dl);
  for (i = 0; i < dlist_size(dl); i++) {
    t->data = tmp[i];
    t = t->next;
  }
  free(tmp);

  return 1;
}
