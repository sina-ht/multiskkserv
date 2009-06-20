/*
 * libstring.c -- String manipulation library
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of multiskkserv.
 *
 * Last Modified: Mon Feb 12 00:20:41 2001.
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

#define REQUIRE_STRING_H
#include "compat.h"

#include "common.h"

#include "libstring.h"

static unsigned char *get(String *);
static unsigned int length(String *);
static int set(String *, const unsigned char *);
static int copy(String *, String *);
static int cat_ch(String *, unsigned char);
static int cat(String *, const unsigned char *);
static int append(String *, String *);
static void shrink(String *, unsigned int);
static String *dup(String *);
static void destroy(String *);

String string_template = {
  len: 0,
  buffer_size: 0,
  buffer: NULL,
  get: get,
  length: length,
  set: set,
  copy: copy,
  cat_ch: cat_ch,
  cat: cat,
  shrink: shrink,
  append: append,
  dup: dup,
  destroy: destroy
};

String *
string_create(void)
{
  String *s;

  if ((s = calloc(1, sizeof(String))) == NULL)
    return NULL;

  memcpy(s, &string_template, sizeof(String));

  return s;
}

/* internal functions */

static void
buffer_free(String *s)
{
  if (string_buffer(s)) {
    free(string_buffer(s));
    string_buffer(s) = NULL;
    string_buffer_size(s) = 0;
  }
}

static int
buffer_alloc(String *s, unsigned int size)
{
  if (string_buffer_size(s) >= size)
    return 1;

  buffer_free(s);

  if ((string_buffer(s) = calloc(1, size)) == NULL)
    return 0;

  string_buffer_size(s) = size;

  return 1;
}

static int
str_alloc(String *s, unsigned int size)
{
  return buffer_alloc(s, size + 1);
}

static int
buffer_increase(String *s, unsigned int size)
{
  char *tmp;
  unsigned int newsize;

  if (string_buffer_size(s)) {
    newsize = string_buffer_size(s) + size;
    if ((tmp = realloc(string_buffer(s), newsize)) == NULL)
      return 0;
  } else {
    newsize = size + 1;
    if ((tmp = calloc(1, newsize)) == NULL)
      return 0;
  }

  string_buffer(s) = tmp;
  string_buffer_size(s) = newsize;

  return 1;
}

/* methods */

static unsigned char *
get(String *s)
{
  return string_buffer(s);
}

static unsigned int
length(String *s)
{
  return string_length(s);
}

static int
set(String *s, const unsigned char *p)
{
  unsigned int l;

  l = strlen(p);
  if (!str_alloc(s, l))
    return 0;
  strcpy(string_buffer(s), p);
  string_length(s) = l;

  return 1;
}

static int
copy(String *s1, String *s2)
{
  if (!str_alloc(s1, string_length(s2)))
    return 0;
  strcpy(string_buffer(s1), string_buffer(s2));
  string_length(s1) = string_length(s2);

  return 1;
}

static int
cat_ch(String *s, unsigned char c)
{
  if (!buffer_increase(s, 1))
    return 0;
  string_buffer(s)[string_length(s)] = c;
  string_buffer(s)[string_length(s) + 1] = '\0';
  string_length(s)++;

  return 1;
}

static int
cat(String *s, const unsigned char *p)
{
  if (!buffer_increase(s, strlen(p)))
    return 0;
  strcat(string_buffer(s), p);
  string_length(s) += strlen(p);

  return 1;
}

static int
append(String *s1, String *s2)
{
  if (!buffer_increase(s1, string_length(s2)))
    return 0;
  strcat(string_buffer(s1), string_buffer(s2));
  string_length(s1) += string_length(s2);

  return 1;
}

static void
shrink(String *s, unsigned int l)
{
  if (string_length(s) <= l)
    return;
  string_length(s) = l;
  string_buffer(s)[l] = '\0';
}

static String *
dup(String *s)
{
  String *new;

  if ((new = string_create()) == NULL)
    return NULL;
  if (!set(new, string_buffer(s))) {
    destroy(new);
    return NULL;
  }

  return new;
}

static void
destroy(String *s)
{
  buffer_free(s);
  free(s);
}
