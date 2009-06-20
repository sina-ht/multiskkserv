/*
 * libstring.h -- String manipulation library header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of multiskkserv.
 *
 * Last Modified: Mon Feb 12 00:21:00 2001.
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

#ifndef _LIBSTRING_H
#define _LIBSTRING_H

typedef struct _string String;

struct _string {
  unsigned int len;
  unsigned int buffer_size;
  unsigned char *buffer;

  unsigned char *(*get)(String *);
  unsigned int (*length)(String *);
  int (*set)(String *, const unsigned char *);
  int (*copy)(String *, String *);
  int (*cat_ch)(String *, unsigned char);
  int (*cat)(String *, const unsigned char *);
  int (*append)(String *, String *);
  void (*shrink)(String *, unsigned int);
  String *(*dup)(String *);
  void (*destroy)(String *);
};

#define string_buffer_size(s) (s)->buffer_size
#define string_buffer(s) (s)->buffer
#define string_length(s) (s)->len

#define string_get(s) (s)->get((s))
#define string_set(s, p) (s)->set((s), (p))
#define string_copy(s1, s2) (s1)->copy((s1), (s2))
#define string_cat_ch(s, c) (s)->cat_ch((s), (c))
#define string_cat(s, p) (s)->cat((s), (p))
#define string_append(s1, s2) (s1)->append((s1), (s2))
#define string_shrink(s, l) (s)->shrink((s), l)
#define string_dup(s) (s)->dup(s)
#define string_destroy(s) (s)->destroy(s)

String *string_create(void);

#endif
