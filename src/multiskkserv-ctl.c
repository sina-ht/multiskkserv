/*
 * multiskkserv-ctl.c -- multiskkserv control utility
 * (C)Copyright 2001, 2002 by Hiroshi Takekawa
 * This file is part of multiskkserv.
 *
 * Last Modified: Sun Jan 13 05:29:31 2002.
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
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>

#include <sys/param.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#ifndef HAVE_GETADDRINFO
#  include "getaddrinfo.h"
#endif

#include "dlist.h"
#include "libstring.h"

#include "multiskkserv.h"
#include "getopt-support.h"

#define BIND_ERROR 1
#define ACCEPT_ERROR 2
#define NO_DICTIONARY_ERROR 3
#define MEMORY_ERROR 4
#define INVALID_PORT_ERROR 5
#define INVALID_NUMBER_ERROR 6
#define INVALID_FAMILY_ERROR 7

static Option options[] = {
  { "help",     'h', _NO_ARGUMENT,       "Show help message." },
  { "server",   's', _REQUIRED_ARGUMENT, "Specify which ip to listen." },
  { "port",     'p', _REQUIRED_ARGUMENT, "Specify which port to listen." },
  { "family",   'f', _REQUIRED_ARGUMENT, "Specify address family: INET or INET6 or UNSPEC." },
  { NULL }
};

static int
socket_connect(char *remote, char **sstr, int port, char *service, int family)
{
  String *s;
  struct addrinfo hints, *res0, *res;
  int sock;
  int opt, gaierr;
  int try_default_portnum;
  char *servername;
  char sbuf[NI_MAXSERV];
  char ipbuf[INET6_ADDRSTRLEN];
  char selfname[MAXHOSTNAMELEN + 1];

  s = string_create();

  gethostname(selfname, MAXHOSTNAMELEN);
  selfname[MAXHOSTNAMELEN] = '\0';
  string_cat(s, selfname);
  string_cat(s, ":");
  string_cat(s, remote);
  string_cat(s, ":");

  if (port > -1) {
    snprintf(sbuf, sizeof(sbuf), "%d", port);
    sbuf[sizeof(sbuf) - 1] = '\0';
    try_default_portnum = 0;
  } else {
    strncpy(sbuf, SKKSERV_SERVICE, sizeof(sbuf));
    try_default_portnum = 1;
  }

  for (;;) {
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;
    hints.ai_socktype = SOCK_STREAM;
    res0 = NULL;
    if ((gaierr = getaddrinfo(remote, sbuf, &hints, &res0))) {
      if (gaierr == EAI_SERVICE && try_default_portnum) {
	snprintf(sbuf, sizeof(sbuf), "%d", SKKSERV_PORT);
	sbuf[sizeof(sbuf) - 1] = '\0';
	try_default_portnum = 0;
	continue;
      }
#ifdef HAVE_GETADDRINFO
      fprintf(stderr, PROGNAME "-ctl: getaddrinfo: %s(gaierr = %d)\n", gai_strerror(gaierr), gaierr);
#endif
      return -2;
    }
    for (res = res0; res; res = res->ai_next) {
      if (res->ai_family != AF_INET && res->ai_family != AF_INET6)
	continue;
      if ((sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
	continue;
      ipbuf[0] = '\0';
#ifdef HAVE_GETNAMEINFO
      if ((gaierr = getnameinfo(res->ai_addr, res->ai_addrlen, ipbuf, sizeof(ipbuf),
				NULL, 0, NI_NUMERICHOST))) {
	fprintf(stderr, PROGNAME "-ctl: getnameinfo: %s\n", gai_strerror(gaierr));
	if (res0)
	  freeaddrinfo(res0);
	close(sock);
	return -3;
      }
#else
      {
	struct sockaddr_in *sp_v4 = (struct sockaddr_in *)&res->ai_addr;
	strncpy(ipbuf, inet_ntoa(sp_v4->sin_addr), sizeof(ipbuf));
      }
#endif
      ipbuf[sizeof(ipbuf) - 1] = '\0';
      opt = 1;
      setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
      if (connect(sock, res->ai_addr, res->ai_addrlen)) {
	perror(PROGNAME "-ctl: connect");
	close(sock);
	sock = -1;
	continue;
      }
      string_cat(s, ipbuf);
      string_cat(s, ":");
      break;
    }
    if (res0)
      freeaddrinfo(res0);
    break;
  }

  string_cat(s, " ");
  *sstr = strdup(string_get(s));
  string_destroy(s);

  return sock;
}

static void
show_stat(char *remote, int port, int family)
{
  int sock;
  char *sstr;
  char rbuf[SKKSERV_REQUEST_SIZE];

  if ((sock = socket_connect(remote, &sstr, port, (char *)SKKSERV_SERVICE, family)) < 0) {
    fprintf(stderr, __FUNCTION__ ": cannot make a connection.\n");
    return;
  }

  snprintf(rbuf, sizeof(rbuf), "S\n");
  write(sock, rbuf, strlen(rbuf));
  if (read(sock, rbuf, sizeof(rbuf)) > 0) {
    int nconns, nactives;

    sscanf(rbuf, "S%d:%d", &nconns, &nactives);
    printf("%d total connections, %d connections active.\n", nconns, nactives);
  }

  free(sstr);
  close(sock);
}

static void
usage(void)
{
  printf(PROGNAME "-ctl version " VERSION "\n");
  printf("(C)Copyright 2001, 2002 by Hiroshi Takekawa\n\n");
  printf("usage: multiskkserv-ctl [options] ['stat']\n");

  printf("Options:\n");
  print_option_usage(options);
}

int
main(int argc, char **argv)
{
  extern char *optarg;
  extern int optind;
  char *optstr;
  char *servername = NULL;
  char *serverstring;
  char *chrootdir = NULL;
  int i, ch;
  int port = -1;
  int family = AF_INET;

  optstr = gen_optstring(options);
  while ((ch = getopt(argc, argv, optstr)) != -1) {
    switch (ch) {
    case 'h':
      usage();
      return 0;
    case 's':
      servername = strdup(optarg);
      break;
    case 'p':
      port = atoi(optarg);
      if (port < 0 || port > 65535) {
	fprintf(stderr, "Invalid port number(%d).\n", port);
	return INVALID_PORT_ERROR;
      }
      break;
    case 'f':
      if (strcasecmp("INET", optarg) == 0)
	family = AF_INET;
      else if (strcasecmp("INET6", optarg) == 0)
	family = AF_INET6;
      else if (strcasecmp("UNSPEC", optarg) == 0)
	family = AF_UNSPEC;
      else if (strcasecmp("4", optarg) == 0)
	family = AF_INET;
      else if (strcasecmp("6", optarg) == 0)
	family = AF_INET6;
      else if (strcasecmp("IPv4", optarg) == 0)
	family = AF_INET;
      else if (strcasecmp("IPv6", optarg) == 0)
	family = AF_INET6;
      else {
	fprintf(stderr, "Invalid family(%s).\n", optarg);
	return INVALID_FAMILY_ERROR;
      }
      break;
    default:
      usage();
      return 0;
    }
  }
  free(optstr);

  if (optind == argc) {
    usage();
    return 0;
  }

  if (strcasecmp(argv[optind], "help") == 0) {
    usage();
  } else if (strcasecmp(argv[optind], "stat") == 0) {
    show_stat(servername, port, family);
  } else {
    usage();
    return 1;
  }

  return 0;
}
