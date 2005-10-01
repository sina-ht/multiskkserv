/*
 * common.h -- common header file, which is included by almost all files.
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Sep 28 23:01:53 2005.
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
#define show_message_fn(format, args...) printf("%s" format, __FUNCTION__ , ## args)
#define show_message_fnc(format, args...) printf("%s: " format, __FUNCTION__ , ## args)
#define warning(format, args...) printf("Warning: " format, ## args)
#define warning_fn(format, args...) printf("Warning: %s" format, __FUNCTION__ , ## args)
#define warning_fnc(format, args...) printf("Warning: %s: " format, __FUNCTION__ , ## args)
#define err_message(format, args...) fprintf(stderr, "Error: " format, ## args)
#define err_message_fn(format, args...) fprintf(stderr, "Error: %s" format, __FUNCTION__ , ## args)
#define err_message_fnc(format, args...) fprintf(stderr, "Error: %s: " format, __FUNCTION__ , ## args)

#if !defined(unlikely)
#if __GNUC__ == 2 && __GNUC_MINOR__ < 96
#define likely(x)   (x)
#define unlikely(x) (x)
#else
#define likely(x)   __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)
#endif
#endif

#define __fatal(msg, format, args...) \
  do {fprintf(stderr, "%s" format, msg, ## args); raise(SIGABRT); exit(1);} while (0)
#define fatal(format, args...) __fatal(PACKAGE " FATAL ERROR: ", format, ## args)
#define fatal_perror(msg) do { perror(msg); raise(SIGABRT); exit(1); } while (0)
#define bug(format, args...) __fatal(PACKAGE " BUG: ", format, ## args) 
#define bug_on(cond) do { if (unlikely(cond)) __fatal(PACKAGE " BUG: cond: ", "%s", #cond); } while (0)

#ifdef DEBUG
#  define PROGNAME PACKAGE "-debug"
#  define debug_message(format, args...) fprintf(stderr, format, ## args)
#  define debug_message_fn(format, args...) fprintf(stderr, "%s" format, __FUNCTION__ , ## args)
#  define debug_message_fnc(format, args...) fprintf(stderr, "%s: " format, __FUNCTION__ , ## args)
#else
#  define PROGNAME PACKAGE
#  define debug_message(format, args...)
#  define debug_message_fn(format, args...)
#  define debug_message_fnc(format, args...)
#endif

#ifdef WITH_DMALLOC
#  include <dmalloc.h>
#endif
