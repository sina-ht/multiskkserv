/*
 * common.h -- common header file, which is included by almost all files.
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Feb  2 13:57:13 2001.
 * $Id$
 *
 * Enfle is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Enfle is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

/* jconfig.h defines this. */
#ifndef CONFIG_H_INCLUDED
#  undef HAVE_STDLIB_H
#endif

/* If compat.h didn't include, include config.h here. */
#ifdef HAVE_CONFIG_H
# ifndef CONFIG_H_INCLUDED
#  include "config.h"
#  define CONFIG_H_INCLUDED
# endif
#endif

#include <stdio.h>
#define show_message(format, args...) printf(format, ## args)
#define warning(format, args...) printf("warning" format, ## args)

#ifdef REQUIRE_FATAL
#include <stdarg.h>
static void fatal(int, const char *, ...) __attribute__ ((noreturn));
static void
fatal(int code, const char *format, ...)
{
  va_list args;

  va_start(args, format);
  fprintf(stderr, "*** " PACKAGE " FATAL: ");
  vfprintf(stderr, format, args);
  va_end(args);

  exit(code);
}
#endif

#ifdef REQUIRE_FATAL_PERROR
static inline void fatal_perror(int, const char *) __attribute__ ((noreturn));
static inline void
fatal_perror(int code, const char *msg)
{
  perror(msg);
  exit(code);
}
#endif

#ifdef DEBUG
#  define PROGNAME PACKAGE "-debug"
#  define debug_message(format, args...) fprintf(stderr, format, ## args)
#  define IDENTIFY_BEFORE_OPEN
#  define IDENTIFY_BEFORE_LOAD
#  define IDENTIFY_BEFORE_PLAY
#else
#  define PROGNAME PACKAGE
#  define debug_message(format, args...)
/* #  define IDENTIFY_BEFORE_OPEN */
/* #  define IDENTIFY_BEFORE_LOAD */
/* #  define IDENTIFY_BEFORE_PLAY */
#endif

#ifdef WITH_DMALLOC
#  include <dmalloc.h>
#endif
