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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifndef HAVE_GETADDRINFO

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#ifdef HAVE_NETINET6_IN6_H
# include <netinet6/in6.h>
#endif /* HAVE_NETINET6_IN6_H */

#include "getaddrinfo.h"

int getaddrinfo (hostname, servname, hints, res)
  const char *hostname;
  const char *servname;
  const struct addrinfo *hints;
  struct addrinfo **res;
{
  struct hostent *host = NULL;
  struct servent *serv = NULL;
  struct protoent *proto;
  int port = 0;

#if (defined (HAVE_SOCKADDR_IN6) && defined (INET6))
  struct sockaddr_in6 *sin =
    (struct sockaddr_in6 *) calloc (1, sizeof (struct sockaddr_in6));
#else /* (defined (HAVE_SOCKADDR_IN6) && defined (INET6)) */
  struct sockaddr_in *sin =
    (struct sockaddr_in *) calloc (1, sizeof (struct sockaddr_in));
#endif /* !(defined (HAVE_SOCKADDR_IN6) && defined (INET6)) */

  struct addrinfo *ai = *res = 
    (struct addrinfo *) calloc (1, sizeof (struct addrinfo));
  
  if ((~ hints->ai_flags & AI_PASSIVE) && hostname && 
      (host = gethostbyname (hostname)) == NULL) {
    perror ("gethostbyname");
    return EAI_NONAME;
  }

  if (hints->ai_protocol && 
      (proto = getprotobynumber (hints->ai_protocol)) == NULL) {
    perror ("getprotobynumber");
    return EAI_NONAME;
  }

  if (servname) 
    if (isdigit (servname[0]))
      port = atoi (servname);
    else {
      if ((serv = getservbyname (servname, proto->p_name)) == NULL) {
	perror ("getservbyname");
	return EAI_NONAME;
      }
      port = serv->s_port;
    }
  
#if (defined (HAVE_SOCKADDR_IN6) && defined (INET6))
  if (host)
    memcpy (&sin->sin6_addr, host->h_addr, host->h_length);
  sin->sin6_port = htons (port);
#else /* (defined (HAVE_SOCKADDR_IN6) && defined (INET6)) */
  if (host)
    memcpy (&sin->sin_addr, host->h_addr, host->h_length);
  sin->sin_port = htons (port);
#endif /* !(defined (HAVE_SOCKADDR_IN6) && defined (INET6)) */

  if (hints->ai_family == AF_UNSPEC)
    ai->ai_family = host->h_addrtype;
  else
    ai->ai_family = hints->ai_family;
#if (defined (HAVE_SOCKADDR_IN6) && defined (INET6))
  sin->sin6_family = ai->ai_family;
#else /* (defined (HAVE_SOCKADDR_IN6) && defined (INET6)) */
  sin->sin_family = ai->ai_family;
#endif /* !(defined (HAVE_SOCKADDR_IN6) && defined (INET6)) */

  ai->ai_protocol = hints->ai_protocol;
  ai->ai_socktype = hints->ai_socktype;
  ai->ai_addrlen = sizeof (*sin);
  ai->ai_addr = (struct sockaddr *)sin;

  return 0;
}

void freeaddrinfo (ai)
     struct addrinfo *ai;
{
  struct addrinfo *p;

  while (ai != NULL) {
    p = ai;
    ai = ai->ai_next;
    free (p);
  }
}

#endif /* HAVE_GETADDRINFO */
