/*
 * multiskkserv.h -- simple skk multi-dictionary server
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of multiskkserv.
 *
 * Last Modified: Fri Dec  7 22:01:57 2001.
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

#ifndef _MULTISKKSERV_H
#define _MULTISKKSERV_H

#define SKKSERV_SERVICE      "skkserv"
#define SKKSERV_WORD_SIZE    1023
#define SKKSERV_RESULT_SIZE  2048
#define SKKSERV_REQUEST_SIZE (SKKSERV_WORD_SIZE + 1)

#define SKKSERV_PORT         1178 /* can be specified by -p */
#define SKKSERV_BACKLOG      8    /* can be specified by -b */

#define SKKSERV_MAX_THREADS  16   /* Should be specified via option? */
#define SKKSERV_EXTENDED     1    /* This enables the statistic query. */

/* skkserv protocol */
#define SKKSERV_C_END       '0'
#define SKKSERV_C_REQUEST   '1'
#define SKKSERV_C_VERSION   '2'
#define SKKSERV_C_HOST      '3'
#ifdef SKKSERV_EXTENDED
# define SKKSERV_C_STAT      'S'
#endif

#define SKKSERV_S_ERROR     '0'
#define SKKSERV_S_FOUND     '1'
#define SKKSERV_S_NOT_FOUND '4'
#define SKKSERV_S_FULL      '9'
#ifdef SKKSERV_EXTENDED
# define SKKSERV_S_STAT      'S'
#endif

#endif
