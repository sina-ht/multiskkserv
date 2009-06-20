/*
 * getaddrinfo(2) emulation.
 * Copyright (C) 1988, 1989, 1992, 1993 Free Software Foundation, Inc.

This file is not part of any package.

GNU Emacs is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Emacs is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Emacs; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
 */

struct addrinfo {
  int ai_flags;
  int ai_family;
  int ai_socktype;
  int ai_protocol;
  size_t ai_addrlen;
  char *ai_canonname;
  struct sockaddr *ai_addr;
  struct addrinfo *ai_next; 
};

/* Possible values for `ai_flags' field in `addrinfo' structure.  */
# define AI_PASSIVE     1       /* Socket address is intended for `bind'.  */
# define AI_CANONNAME   2       /* Request for canonical name.  */
# define AI_NUMERICHOST 3       /* Don't use name resolution.  */

/* Error values for `getaddrinfo' function.  */
#define EAI_BADFLAGS   -1      /* Invalid value for `ai_flags' field.  */
#define EAI_NONAME     -2      /* NAME or SERVICE is unknown.  */
#define EAI_AGAIN      -3      /* Temporary failure in name resolution.  */
#define EAI_FAIL       -4      /* Non-recoverable failure in name res.  */
#define EAI_NODATA     -5      /* No address associated with NAME.  */
#define EAI_FAMILY     -6      /* `ai_family' not supported.  */
#define EAI_SOCKTYPE   -7      /* `ai_socktype' not supported.  */
#define EAI_SERVICE    -8      /* SERVICE not supported for `ai_socktype'.  */
#define EAI_ADDRFAMILY -9      /* Address family for NAME not supported.  */
#define EAI_MEMORY     -10     /* Memory allocation failure.  */
#define EAI_SYSTEM     -11     /* System error returned in `errno'.  */

#define NI_MAXHOST      1025
#define NI_MAXSERV      32

#define NI_NUMERICHOST 1       /* Don't try to look up hostname.  */
#define NI_NUMERICSERV 2       /* Don't convert port number to name.  */
#define NI_NOFQDN      4       /* Only return nodename portion.  */
#define NI_NAMEREQD    8       /* Don't return numeric addresses.  */
#define NI_DGRAM       16      /* Look up UDP service rather than TCP.  */

#define INET6_ADDRSTRLEN 46
typedef unsigned int socklen_t;

extern int getaddrinfo (const char *, const char *, const struct addrinfo *, 
			struct addrinfo **);

extern void freeaddrinfo (struct addrinfo *ai);
