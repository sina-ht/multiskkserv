/*
 * multiskkserv.c -- simple skk multi-dictionary server
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of multiskkserv.
 *
 * Last Modified: Mon Feb 12 03:36:54 2001.
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
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <cdb.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#ifndef HAVE_GETADDRINFO
#  include "getaddrinfo.h"
#endif

#include "dlist.h"
#include "libstring.h"

/* as you like */
#define SKKSERV_SERVICE "skkserv"
#define SKKSERV_PORT 1178
#define SKKSERV_BACKLOG 8
#define SKKSERV_WORD_SIZE 1023
#define SKKSERV_RESULT_SIZE 2048
#define COMMAND_BUFFER_SIZE (SKKSERV_WORD_SIZE + 1)

/* skkserv protocol */
#define SKKSERV_C_END       '0'
#define SKKSERV_C_REQUEST   '1'
#define SKKSERV_C_VERSION   '2'
#define SKKSERV_C_HOST      '3'

#define SKKSERV_S_ERROR     '0'
#define SKKSERV_S_FOUND     '1'
#define SKKSERV_S_NOT_FOUND '4'
#define SKKSERV_S_FULL      '9'

typedef enum _argument_requirement {
  _NO_ARGUMENT,
  _REQUIRED_ARGUMENT,
  _OPTIONAL_ARGUMENT
} ArgumentRequirement;

typedef struct _option {
  const char *longopt; /* not supported so far */
  char opt;
  ArgumentRequirement argreq;
  const unsigned char *description;
} Option;

static Option options[] = {
  { "help",     'h', _NO_ARGUMENT,       "Show help message." },
  { "server",   's', _REQUIRED_ARGUMENT, "Specify which ip to listen." },
  { "port",     'p', _REQUIRED_ARGUMENT, "Specify which port to listen." },
  { "nodaemon", 'n', _NO_ARGUMENT,       "To be invoked from inetd, tcpserver or such." },
  { NULL }
};

typedef struct _dictionary {
  char *path;
  int fd;
  pthread_mutex_t mutex;
  struct cdb cdb;
} Dictionary;

typedef struct _skkconnection {
  char *peername;
  Dlist *dic_list;
  int close_after_use;
  int in;
  int out;
} SkkConnection;

static void
usage(void)
{
  int i;

  printf(PROGNAME " version " VERSION "\n");
  printf("(C)Copyright 2000, 2001 by Hiroshi Takekawa\n\n");
  printf("usage: multiskkserv [options] [path...]\n");

  printf("Options:\n");
  i = 0;
  while (options[i].longopt != NULL) {
    printf(" %c(%s): \t%s\n",
	   options[i].opt, options[i].longopt, options[i].description);
    i++;
  }
}

static char *
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

static int
prepare_listen(char *sname, int port, char *service)
{
  struct addrinfo hints, *res0, *res;
  int gs;
  int opt, gaierr;
  int try_default_portnum;
  char *servername;
  char sbuf[NI_MAXSERV];
  char ipbuf[INET6_ADDRSTRLEN];

  servername = sname ? sname : strdup("localhost");
  if (port > -1) {
    sprintf(sbuf, "%d", port);
    sbuf[sizeof(sbuf) - 1] = '\0';
    try_default_portnum = 0;
  } else {
    strncpy(sbuf, SKKSERV_SERVICE, sizeof(sbuf));
    try_default_portnum = 1;
  }
  for (;;) {
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    res = res0 = NULL;
    if ((gaierr = getaddrinfo(servername, sbuf, &hints, &res0))) {
      if (gaierr == EAI_SERVICE && try_default_portnum) {
	sprintf(sbuf, "%d", port);
	sbuf[sizeof(sbuf) - 1] = '\0';
	try_default_portnum = 0;
	continue;
      }
#ifdef HAVE_GETADDRINFO
      fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(gaierr));
#endif
      return -2;
    }
    for (res = res0; res; res = res->ai_next) {
      if ((gs = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
	continue;
#if 0
#ifdef HAVE_GETNAMEINFO
      ipbuf[0] = '\0';
      if ((gaierr = getnameinfo(res->ai_addr, res->ai_addrlen, ipbuf, sizeof(ipbuf),
				NULL, 0, NI_NUMERICHOST))) {
	fprintf(stderr, "getnameinfo(): %s\n", gai_strerror(gaierr));
	if (res0)
	  freeaddrinfo(res0);
	return -3;
      }
      ipbuf[sizeof(ipbuf) - 1] = '\0';
#endif
#endif
      opt = 1;
      setsockopt(gs, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
      if (bind(gs, res->ai_addr, res->ai_addrlen)) {
	close(gs);
	gs = -1;
	continue;
      }
      if (listen(gs, SKKSERV_BACKLOG)) {
	close(gs);
	gs = -1;
	continue;
      }
      break;
    }
    if (res0)
      freeaddrinfo(res0);
    break;
  }

  return gs;
}

static Dictionary *
open_dictionary(char *path)
{
  Dictionary *dic;

  if ((dic = calloc(1, sizeof(Dictionary))) == NULL) {
    fprintf(stderr, "No enough memory.\n");
    return NULL;
  }
  if ((dic->path = strdup(path)) == NULL)
    return NULL;
  if ((dic->fd = open(path, O_RDONLY)) == -1) {
    perror("multiskkserv: ");
    return NULL;
  }
  cdb_init(&dic->cdb, dic->fd);
  pthread_mutex_init(&dic->mutex, NULL);

  return dic;
}

static void
search_dictionaries(int out, Dlist *dic_list, char *cbuf)
{
  Dlist_data *dd;
  char result;
  char str[SKKSERV_WORD_SIZE];
  char buf[SKKSERV_RESULT_SIZE];
  char *end;
  int emit_result = 0;
  int r, len, rlen = 0;

  if ((end = (char *)strchr(cbuf + 1, ' ')) == NULL) {
    cbuf[0] = SKKSERV_S_ERROR;
    write(out, cbuf, strlen(cbuf));
    return;
  }
  len = end - (cbuf + 1);
  memcpy(str, cbuf + 1, len);
  str[len] = '\0';

  debug_message(__FUNCTION__ ": str %s\n", str);

  dlist_iter(dic_list,dd) {
    Dictionary *dic = dlist_data(dd);

    pthread_mutex_lock(&dic->mutex);
    cdb_findstart(&dic->cdb);
    if ((r = cdb_findnext(&dic->cdb, str, len)) == -1) {
      fprintf(stderr, "cdb_findnext() failed.\n");
      if (!emit_result) {
	cbuf[0] = SKKSERV_S_ERROR;
	write(out, cbuf, strlen(cbuf));
      }
      pthread_mutex_unlock(&dic->mutex);
      return;
    }
    if (r) {
      debug_message(__FUNCTION__ ": %s found\n", str);
      if (!emit_result) {
	if (1 + cdb_datalen(&dic->cdb) + 2 > SKKSERV_RESULT_SIZE) {
	  fprintf(stderr, "Truncated: %s\n", str);
	  r = SKKSERV_RESULT_SIZE - 3;
	} else {
	  r = cdb_datalen(&dic->cdb);
	}
	if (cdb_read(&dic->cdb, buf + 1, r, cdb_datapos(&dic->cdb)) == -1) {
	  fprintf(stderr, "cdb_read() failed.\n");
	  cbuf[0] = SKKSERV_S_ERROR;
	  write(out, cbuf, strlen(cbuf));
	  pthread_mutex_unlock(&dic->mutex);
	  return;
	}
	if (buf[r] == '/') {
	  buf[r] = '\0';
	  rlen = r;
	} else {
	  buf[r + 1] = '\0';
	  rlen = r + 1;
	}
	buf[0] = SKKSERV_S_FOUND;
	emit_result = 1;
      } else {
	if (rlen + cdb_datalen(&dic->cdb) + 2 > SKKSERV_RESULT_SIZE) {
	  fprintf(stderr, "Truncated: %s\n", str);
	  r = SKKSERV_RESULT_SIZE - rlen - 2;
	}
	if (cdb_read(&dic->cdb, buf + rlen, r, cdb_datapos(&dic->cdb)) == -1) {
	  buf[rlen] = '\0';
	  continue;
	}
	if (buf[rlen + r] == '/') {
	  buf[rlen + r] = '\0';
	  rlen += r;
	} else {
	  buf[rlen + r + 1] = '\0';
	  rlen += r + 1;
	}
      }
    }
    pthread_mutex_unlock(&dic->mutex);
  }

  if (rlen) {
    buf[rlen] = '/';
    buf[rlen + 1] = '\n';
    buf[rlen + 2] = '\0';
    write(out, buf, strlen(buf));
  } else {
    cbuf[0] = SKKSERV_S_NOT_FOUND;
    write(out, cbuf, strlen(cbuf));
  }
}

static void *
skkserver(void *arg)
{
  SkkConnection *conn = arg;
  char cbuf[COMMAND_BUFFER_SIZE];
  int read_size;

  debug_message(__FUNCTION__ ": client = %s\n", conn->peername);

  while ((read_size = read(conn->in, cbuf, COMMAND_BUFFER_SIZE - 1)) > 0) {
    cbuf[read_size] = '\0';
    switch (cbuf[0]) {
    case SKKSERV_C_END:
      goto end;
    case SKKSERV_C_REQUEST:
      search_dictionaries(conn->out, conn->dic_list, cbuf);
      break;
    case SKKSERV_C_VERSION:
      write(conn->out, VERSION, strlen(VERSION));
      break;
    case SKKSERV_C_HOST:
      write(conn->out, conn->peername, strlen(conn->peername));
      break;
    }
  }

 end:
  free(conn->peername);

  if (conn->close_after_use) {
    close(conn->in);
    if (conn->in != conn->out)
      close(conn->out);
  }

  pthread_exit((void *)0);
}

int
main(int argc, char **argv)
{
  extern char *optarg;
  extern int optind;
  Dlist *dic_list;
  Dictionary *dic;
  SkkConnection skkconn;
  pthread_t thread;
  char *optstr;
  char *servername = NULL;
  int i, ch;
  int port = -1;
  int daemon = 1;

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
      break;
    case 'n':
      daemon = 0;
      break;
    default:
      usage();
      return 1;
    }
  }

  dic_list = dlist_create();
  for (i = optind; i < argc; i++) {
    if ((dic = open_dictionary(argv[i])) == NULL)
      fprintf(stderr, "Cannot open dictionary %s\n", argv[i]);
    else
      dlist_add(dic_list, dic);
  }
  if (!dlist_size(dic_list)) {
    fprintf(stderr, "No dictionary.\n");
    return 4;
  }

  if (daemon) {
    int gs;

    if ((gs = prepare_listen(servername, port, (char *)SKKSERV_SERVICE)) < 0) {
      fprintf(stderr, "Cannot bind\n");
      return 2;
    }

    /* daemon loop */
    for (;;) {
      struct sockaddr sa;
      struct sockaddr sp;
      socklen_t salen, splen;
      char ipbuf[INET6_ADDRSTRLEN];
      int gaierr;
      int s;

      salen = sizeof(sa);
      if ((s = accept(gs, &sa, &salen)) == -1) {
	perror("accept: ");
	close(gs);
	return 3;
      }

      splen = sizeof(sp);
      if ((getpeername(s, &sp, &splen)) < 0) {
	perror("getpeername: ");
	memset(&sp, 0, sizeof(sp));
      }
      ipbuf[0] = '\0';
#ifdef HAVE_GETNAMEINFO
      if ((gaierr = getnameinfo(&sp, splen, ipbuf, sizeof(ipbuf),
				NULL, 0, NI_NUMERICHOST))) {
	fprintf(stderr, "getnameinfo(): %s\n", gai_strerror(gaierr));
	skkconn.peername = strdup("UNKNOWN");
      } else {
	ipbuf[sizeof(ipbuf) - 1] = '\0';
	skkconn.peername = strdup(ipbuf);
      }
#else
      {
	struct sockaddr_in *sp_v4 = (struct sockaddr_in *)&sp;
	skkconn.peername = strdup(inet_ntoa(sp_v4->sin_addr));
      }
#endif
      skkconn.dic_list = dic_list;
      skkconn.close_after_use = 1;
      skkconn.in = s;
      skkconn.out = s;
      pthread_create(&thread, NULL, skkserver, (void *)&skkconn);
    }
  } else {
    void *ret;

    skkconn.dic_list = dic_list;
    skkconn.close_after_use = 0;
    skkconn.in = 0;
    skkconn.out = 1;
    pthread_create(&thread, NULL, skkserver, (void *)&skkconn);
    pthread_join(thread, &ret);
  }

  return 0;
}
