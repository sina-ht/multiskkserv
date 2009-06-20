/*
 * dlist.c -- double-linked list data structure
 * (C)Copyright 1998, 99, 2000, 2001 by Hiroshi Takekawa
 * This file is part of multiskkserv.
 *
 * Last Modified: Mon Feb 12 00:19:39 2001.
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

static int attach(Dlist *, Dlist_data *, Dlist_data *);
static Dlist_data *insert(Dlist *, Dlist_data *, void *);
static Dlist_data *add(Dlist *, void *);
static Dlist_data *add_str(Dlist *, char *);
static int detach(Dlist *, Dlist_data *);
static int delete_item(Dlist *, Dlist_data *);
static int move_to_top(Dlist *, Dlist_data *);
static void set_compfunc(Dlist *, Dlist_compfunc);
static int do_sort(Dlist *);
static int destroy(Dlist *, int);

static Dlist template = {
  attach: attach,
  insert: insert,
  add: add,
  add_str: add_str,
  detach: detach,
  delete_item: delete_item,
  move_to_top: move_to_top,
  set_compfunc: set_compfunc,
  do_sort: do_sort,
  destroy: destroy
};

static Dlist_data *
dlist_data_create(Dlist *dl)
{
  Dlist_data *dd;

  if ((dd = calloc(1, sizeof(Dlist_data))) == NULL)
    return NULL;
  dd->dl = dl;

  return dd;
}

Dlist *
dlist_create(void)
{
  Dlist *dl;

  if ((dl = calloc(1, sizeof(Dlist))) == NULL)
    goto error;
  memcpy(dl, &template, sizeof(Dlist));

  if ((dl->guard = dlist_data_create(dl)) == NULL)
    goto error;
  dlist_prev(dl->guard) = dl->guard;
  dlist_next(dl->guard) = dl->guard;

  return dl;

 error:
  if (dl)
    free(dl);

  return NULL;
}

static int
destroy(Dlist *dl, int f)
{
  Dlist_data *dd, *dd_n, *g;

  g = dlist_guard(dl);
  dlist_next(dlist_prev(g)) = NULL;
  dd_n = dlist_next(g);
  free(g);
  for (dd = dd_n; dd; dd = dd_n) {
    dd_n = dlist_next(dd);
    if (f && dlist_data(dd))
      free(dlist_data(dd));
    free(dd);
  }
  free(dl);

  return 1;
}

static int
attach(Dlist *dl, Dlist_data *inserted, Dlist_data *inserted_n)
{
  Dlist_data *inserted_p;

  inserted_p = dlist_prev(inserted_n);

  dlist_next(inserted_p) = inserted;
  dlist_prev(inserted) = inserted_p;
  dlist_next(inserted) = inserted_n;
  dlist_prev(inserted_n) = inserted;

  dlist_size(dl)++;

  return 1;
}

static Dlist_data *
insert(Dlist *dl, Dlist_data *inserted, void *d)
{
  Dlist_data *dd;

  if (dlist_dlist(inserted) != dl) {
    debug_message("inserted(dl: %p, self: %p, data: %p) is not in dl(%p)\n", dlist_dlist(inserted), inserted, dlist_data(inserted), dl);
    raise(SIGABRT);
    //return NULL;
  }

  if ((dd = dlist_data_create(dl)) == NULL)
    return NULL;
  dd->data = d;

  attach(dl, dd, inserted);

  return dd;
}

static Dlist_data *
add(Dlist *dl, void *d)
{
  return insert(dl, dlist_guard(dl), d);
}

static Dlist_data *
add_str(Dlist *dl, char *str)
{
  char *new_str;

  if (str == NULL)
    return NULL;
  if ((new_str = strdup(str)) == NULL)
    return NULL;

  return add(dl, (void *)new_str);
}

static int
detach(Dlist *dl, Dlist_data *dd)
{
  Dlist_data *dd_p, *dd_n;

  if (dlist_guard(dl) == dd)
    return 0;

  dd_p = dlist_prev(dd);
  dd_n = dlist_next(dd);
  dlist_next(dd_p) = dd_n;
  dlist_prev(dd_n) = dd_p;

  dlist_size(dl)--;

  return 1;
}

static int
delete_item(Dlist *dl, Dlist_data *dd)
{
  if (dl == NULL || dd == NULL)
    return 0;
  if (dlist_dlist(dd) != dl)
    return 0;

  if (!detach(dl, dd))
    return 0;

  if (dlist_data(dd))
    free(dlist_data(dd));
  free(dd);

  return 1;
}

static int
move_to_top(Dlist *dl, Dlist_data *dd)
{
  if (dlist_dlist(dd) != dl)
    return 0;
  if (dlist_top(dl) == dd)
    return 1;

  if (!detach(dl, dd))
    return 0;

  attach(dl, dd, dlist_top(dl));

  return 1;
}

static void
set_compfunc(Dlist *dl, Dlist_compfunc cf)
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

static int
do_sort(Dlist *dl)
{
  int i;
  Dlist_data *t;
  void **tmp;

  if (dlist_size(dl) <= 1)
    return 1;

  if ((tmp = calloc(dlist_size(dl), sizeof(void *))) == NULL)
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
