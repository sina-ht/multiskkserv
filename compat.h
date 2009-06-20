/*
 * compat.h -- for compatibility
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Jan  8 19:51:06 2001.
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

#ifdef HAVE_CONFIG_H
# ifndef CONFIG_H_INCLUDED
#  include "config.h"
#  define CONFIG_H_INCLUDED
# endif
#endif

#ifdef REQUIRE_STRING_H
# if STDC_HEADERS
#  include <string.h>
# else
#  ifndef HAVE_MEMCPY
#   define memcpy(d, s, n) bcopy((s), (d), (n))
#   define memmove(d, s, n) bcopy((s), (d), (n))
#  endif
# endif
#endif

#ifdef REQUIRE_UNISTD_H
# ifdef HAVE_UNISTD_H
#  include <sys/types.h>
#  include <unistd.h>
# endif
#endif

#ifdef REQUIRE_DIRENT_H
# if HAVE_DIRENT_H
#  include <dirent.h>
# else
#  define dirent direct
#  if HAVE_SYS_NDIR_H
#   include <sys/ndir.h>
#  endif
#  if HAVE_SYS_DIR_H
#   include <sys/dir.h>
#  endif
#  if HAVE_NDIR_H
#   include <ndir.h>
#  endif
# endif
#endif
