/*
 * getopt-support.c -- getopt support
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of multiskkserv.
 *
 * Last Modified: Fri Dec  7 23:04:39 2001.
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

#include <stdio.h>

#define REQUIRE_STRING_H
#include "compat.h"

#include "libstring.h"
#include "getopt-support.h"

char *
gen_optstring(Option opt[])
{
  int i;
  String *s;
  char *optstr;

  s = string_create();
  i = 0;
  while (opt[i].longopt != NULL) {
    string_cat_ch(s, opt[i].opt);
    switch (opt[i].argreq) {
    case _NO_ARGUMENT:
      break;
    case _REQUIRED_ARGUMENT:
      string_cat_ch(s, ':');
      break;
    case _OPTIONAL_ARGUMENT:
      string_cat(s, "::");
      break;
    }
    i++;
  }

  optstr = strdup(string_get(s));

  string_destroy(s);

  return optstr;
}

void
print_option_usage(Option opt[])
{
  int i;

  i = 0;
  while (opt[i].longopt != NULL) {
    printf(" %c(%s): \t%s\n",
	   opt[i].opt, opt[i].longopt, opt[i].description);
    i++;
  }
}
