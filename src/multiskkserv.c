/*
 * multiskkserv.c -- simple skk multi-dictionary server
 * (C)Copyright 2001-2010 by Hiroshi Takekawa
 * This file is part of multiskkserv.
 *
 * Last Modified: Wed Jan 27 15:16:41 2010.
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

#include <cdb.h>

#ifndef HAVE_GETADDRINFO
#  include "getaddrinfo.h"
#endif

#include "dlist.h"
#include "hash.h"
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
#define CHDIR_ERROR 8
#define CHROOT_ERROR 9

static Option options[] = {
  { "help",     'h', _NO_ARGUMENT,       "Show help message." },
  { "server",   's', _REQUIRED_ARGUMENT, "Specify which ip to listen." },
  { "port",     'p', _REQUIRED_ARGUMENT, "Specify which port to listen." },
  { "backlog",  'b', _REQUIRED_ARGUMENT, "Specify how many backlogs to listen." },
  { "root",     'r', _REQUIRED_ARGUMENT, "chroot() to the specified directory." },
  { "family",   'f', _REQUIRED_ARGUMENT, "Specify address family: INET or INET6 or UNSPEC." },
  { "nodaemon", 'n', _NO_ARGUMENT,       "To be invoked from inetd, tcpserver or such." },
  { "debug",    'd', _NO_ARGUMENT,       "Debug mode (do nothing so far)." },
  { NULL }
};

typedef struct _dictionary {
  char *path;
  int fd;
  pthread_mutex_t mutex;
  struct cdb cdb;
#if defined (USE_TINYCDB)
  int first;
  struct cdb_find cdb_find;
#endif
} Dictionary;

typedef struct _connectionstat {
  pthread_mutex_t mutex;
  unsigned int nconns;
  unsigned int nactives;
} ConnectionStat;

typedef struct _skkconnection {
  char *serverstring;
  char *peername;
  ConnectionStat *stat;
  Dlist *dic_list;
  int close_after_use;
  int in;
  int out;
} SkkConnection;

#if defined(USE_TINYCDB)
#define USE_CDB " with included tinycdb"
#else
#define USE_CDB " with external cdb"
#endif
static void
usage(void)
{
  printf(PROGNAME " version " VERSION USE_CDB "\n");
  printf("(C)Copyright 2001-2010 by Hiroshi Takekawa\n\n");
  printf("usage: multiskkserv [options] [path...]\n");

  printf("Options:\n");
  print_option_usage(options);
}

static int
prepare_listen(char *sname, char **sstr, int port, char *service, int nbacklogs, int family)
{
  String *s;
  struct addrinfo hints, *res0, *res;
  int gs = 0;
  int opt, gaierr;
  int try_default_portnum;
  char *servername;
  char sbuf[NI_MAXSERV];
  char ipbuf[INET6_ADDRSTRLEN];
  char selfname[MAXHOSTNAMELEN + 1];

  s = string_create();

  servername = sname ? sname : strdup("localhost");

  gethostname(selfname, MAXHOSTNAMELEN);
  selfname[MAXHOSTNAMELEN] = '\0';
  string_cat(s, selfname);
  string_cat(s, ":");

  if (port > -1) {
    snprintf(sbuf, sizeof(sbuf), "%d", port);
    sbuf[sizeof(sbuf) - 1] = '\0';
    try_default_portnum = 0;
  } else {
    strncpy(sbuf, service, sizeof(sbuf));
    try_default_portnum = 1;
  }

  for (;;) {
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;
    hints.ai_socktype = SOCK_STREAM;
    res0 = NULL;
    if ((gaierr = getaddrinfo(servername, sbuf, &hints, &res0))) {
      if (gaierr == EAI_SERVICE && try_default_portnum) {
	snprintf(sbuf, sizeof(sbuf), "%d", SKKSERV_PORT);
	sbuf[sizeof(sbuf) - 1] = '\0';
	try_default_portnum = 0;
	continue;
      }
#ifdef HAVE_GETADDRINFO
      err_message_fnc("getaddrinfo(): %s(gaierr = %d)\n", gai_strerror(gaierr), gaierr);
#endif
      return -2;
    }
    for (res = res0; res; res = res->ai_next) {
      if (res->ai_family != AF_INET && res->ai_family != AF_INET6)
	continue;
      if ((gs = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
	continue;
      ipbuf[0] = '\0';
#ifdef HAVE_GETNAMEINFO
      if ((gaierr = getnameinfo(res->ai_addr, res->ai_addrlen, ipbuf, sizeof(ipbuf),
				NULL, 0, NI_NUMERICHOST))) {
	err_message_fnc("getnameinfo(): %s\n", gai_strerror(gaierr));
	if (res0)
	  freeaddrinfo(res0);
	close(gs);
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
      setsockopt(gs, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
      if (bind(gs, res->ai_addr, res->ai_addrlen)) {
	perror(PROGNAME ": bind");
	close(gs);
	gs = -1;
	continue;
      }
      if (listen(gs, nbacklogs)) {
	perror(PROGNAME ": listen");
	close(gs);
	gs = -1;
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

  return gs;
}

static Dictionary *
open_dictionary(char *path)
{
  Dictionary *dic;

  if ((dic = calloc(1, sizeof(Dictionary))) == NULL) {
    err_message_fnc("No enough memory.\n");
    return NULL;
  }

  if ((dic->path = strdup(path)) == NULL)
    return NULL;

  if ((dic->fd = open(path, O_RDONLY)) == -1) {
    perror(PROGNAME ": open");
    return NULL;
  }

  cdb_init(&dic->cdb, dic->fd);
  pthread_mutex_init(&dic->mutex, NULL);

  return dic;
}

static void __cdb_findstart(Dictionary *dic)
{
#if defined(USE_TINYCDB)
  dic->first = 1;
#else
  cdb_findstart(&dic->cdb);
#endif
}

static int __cdb_findnext(Dictionary *dic, void *key, unsigned int keylen)
{
#if defined(USE_TINYCDB)
  if (dic->first) {
    cdb_findinit(&dic->cdb_find, &dic->cdb, key, keylen);
    dic->first = 0;
  }
  return cdb_findnext(&dic->cdb_find);
#else
  return cdb_findnext(&dic->cdb, key, keylen);
#endif
}

/* Must be prime */
#define HASH_SIZE 4099
static void
search_dictionaries(int out, Dlist *dic_list, char *rbuf)
{
  Dlist_data *dd;
  Hash *word_hash;
  char word[SKKSERV_WORD_SIZE];
  char result[SKKSERV_RESULT_SIZE];
  char tmpresult[SKKSERV_RESULT_SIZE];
  char *end;
  int i, p, r, len, rlen, ncandidates;

  if ((end = strchr(rbuf + 1, ' ')) == NULL) {
    rbuf[0] = SKKSERV_S_ERROR;
    write(out, rbuf, strlen(rbuf));
    return;
  }
  len = end - (rbuf + 1);
  memcpy(word, rbuf + 1, len);
  word[len] = '\0';

  word_hash = hash_create(HASH_SIZE);
  rlen = 1;
  ncandidates = 0;
  result[0] = SKKSERV_S_FOUND;
  dlist_iter(dic_list, dd) {
    Dictionary *dic = dlist_data(dd);

    pthread_mutex_lock(&dic->mutex);
    __cdb_findstart(dic);
    if ((r = __cdb_findnext(dic, word, len)) == -1) {
      err_message_fnc("cdb_findnext() failed.\n");
      if (!ncandidates) {
	rbuf[0] = SKKSERV_S_ERROR;
	write(out, rbuf, strlen(rbuf));
      }
      pthread_mutex_unlock(&dic->mutex);
      hash_destroy(word_hash);
      return;
    }
    debug_message_fnc("word %s, len %d, r = %d\n", word, len, r);
    if (r) {
      int dpos = cdb_datapos(&dic->cdb);
      int dlen = cdb_datalen(&dic->cdb);
      debug_message_fnc("%s found, r = %d, dpos = %d, dlen = %d.\n", word, r, dpos, dlen);
      if (rlen + dlen + 2 > SKKSERV_RESULT_SIZE) {
	err_message_fnc("Truncated: %s\n", word);
	r = SKKSERV_RESULT_SIZE - rlen - 2;
      } else {
	r = dlen;
      }
      debug_message_fnc("read %d bytes\n", r);
      if (cdb_read(&dic->cdb, tmpresult, r, dpos) == -1) {
	if (!ncandidates) {
	  err_message_fnc("cdb_read() failed.\n");
	  rbuf[0] = SKKSERV_S_ERROR;
	  write(out, rbuf, strlen(rbuf));
	  pthread_mutex_unlock(&dic->mutex);
	  hash_destroy(word_hash);
	  return;
	} else {
	  result[rlen] = '\0';
	  continue;
	}
      }

      /* Merge */
      p = 0;
      i = 1;
      while (i < r) {
	if (tmpresult[i] == '/') {
	  if (i - p - 1 > 0 &&
	      hash_define_value(word_hash, tmpresult + p + 1, i - p - 1, (void *)1) == 1) {
	    memcpy(result + rlen, tmpresult + p, i - p);
	    rlen += i - p;
	    ncandidates++;
	  }
	  p = i;
	}
	i++;
      }
    }
    pthread_mutex_unlock(&dic->mutex);
  }

  hash_destroy(word_hash);

  if (ncandidates) {
    result[rlen] = '/';
    result[rlen + 1] = '\n';
    result[rlen + 2] = '\0';
    write(out, result, strlen(result));
  } else {
    rbuf[0] = SKKSERV_S_NOT_FOUND;
    write(out, rbuf, strlen(rbuf));
  }
}

static void
close_serverconnection(SkkConnection *conn)
{
  if (conn->close_after_use) {
    close(conn->in);
    if (conn->in != conn->out)
      close(conn->out);
  }
}

static void *
skkserver(void *arg)
{
  SkkConnection *conn = arg;
  char rbuf[SKKSERV_REQUEST_SIZE];
  int read_size;

  debug_message("%s: client = %s\n", __FUNCTION__, conn->peername);

  while ((read_size = read(conn->in, rbuf, SKKSERV_REQUEST_SIZE - 1)) > 0) {
    rbuf[read_size] = '\0';
    switch (rbuf[0]) {
    case SKKSERV_C_END:
      goto end;
    case SKKSERV_C_REQUEST:
      search_dictionaries(conn->out, conn->dic_list, rbuf);
      break;
    case SKKSERV_C_VERSION:
      write(conn->out, VERSION " ", strlen(VERSION) + 1);
      break;
    case SKKSERV_C_HOST:
      write(conn->out, conn->serverstring, strlen(conn->serverstring));
      break;
#ifdef SKKSERV_EXTENDED
    case SKKSERV_C_STAT:
      if (conn->stat) {
	rbuf[0] = SKKSERV_S_STAT;
	snprintf(rbuf + 1, SKKSERV_REQUEST_SIZE - 2, "%d:%d ", conn->stat->nconns, conn->stat->nactives);
	write(conn->out, rbuf, strlen(rbuf));
      } else {
	rbuf[0] = SKKSERV_S_ERROR;
	write(conn->out, rbuf, 1);
      }
      break;
#endif
    default:
      rbuf[0] = SKKSERV_S_ERROR;
      write(conn->out, rbuf, 1);
      break;
    }
  }

 end:
  free(conn->peername);

  close_serverconnection(conn);

  if (conn->stat) {
    pthread_mutex_lock(&conn->stat->mutex);
    conn->stat->nactives--;
    pthread_mutex_unlock(&conn->stat->mutex);
  }

  free(conn);

  return (void *)0;
}

static pthread_t
create_server_thread(SkkConnection *conn, int detached)
{
  pthread_t thread;
  pthread_attr_t attr;
  int i;

  i = 0;
  while (conn->stat && conn->stat->nactives > SKKSERV_MAX_THREADS) {
    sleep(1);
    i++;
    if (i > 30) {
      close_serverconnection(conn);
      return 0;
    }
  }

  pthread_attr_init(&attr);
  if (detached)
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&thread, &attr, skkserver, (void *)conn);

  if (conn->stat) {
    pthread_mutex_lock(&conn->stat->mutex);
    conn->stat->nconns++;
    conn->stat->nactives++;
    pthread_mutex_unlock(&conn->stat->mutex);
  }

  return thread;
}

int
main(int argc, char **argv)
{
  extern char *optarg;
  extern int optind;
  Dlist *dic_list;
  Dictionary *dic;
  SkkConnection *skkconn;
  ConnectionStat stat;
  char *optstr;
  char *servername = NULL;
  char *serverstring = NULL;
  char *chrootdir = NULL;
  int i, ch;
  int port = -1;
  int daemon = 1;
  int nbacklogs = SKKSERV_BACKLOG;
  int family = AF_INET;
  int gs = 0;

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
	err_message_fnc("Invalid port number(%d).\n", port);
	return INVALID_PORT_ERROR;
      }
      break;
    case 'b':
      nbacklogs = atoi(optarg);
      if (nbacklogs < 1 || nbacklogs > 64) {
	err_message_fnc("Invalid number for the number of backlogs(%d).\n", nbacklogs);
	return INVALID_NUMBER_ERROR;
      }
      break;
    case 'r':
      chrootdir = strdup(optarg);
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
	err_message_fnc("Invalid family(%s).\n", optarg);
	return INVALID_FAMILY_ERROR;
      }
      break;
    case 'n':
      daemon = 0;
      break;
    case 'd':
      /* for skk-server-debug, though this seems to be needless. */
      break;
    default:
      usage();
      return 0;
    }
  }
  free(optstr);

  if (daemon) {
    if ((gs = prepare_listen(servername, &serverstring, port, (char *)SKKSERV_SERVICE,
			     nbacklogs, family)) < 0) {
      err_message_fnc("Cannot bind\n");
      return BIND_ERROR;
    }
  }

  if (chrootdir) {
    if (chdir(chrootdir)) {
      perror(PROGNAME ": chdir");
      return CHDIR_ERROR;
    }
    if (chroot(chrootdir)) {
      perror(PROGNAME ": chroot");
      return CHROOT_ERROR;
    }
  }

  dic_list = dlist_create();
  for (i = optind; i < argc; i++) {
    if ((dic = open_dictionary(argv[i])) == NULL)
      err_message_fnc("Cannot open dictionary %s\n", argv[i]);
    else
      dlist_add(dic_list, dic);
  }
  if (!dlist_size(dic_list)) {
    err_message_fnc("No dictionary.\n");
    return NO_DICTIONARY_ERROR;
  }

  if (daemon) {
    stat.nconns = 0;
    stat.nactives = 0;
    pthread_mutex_init(&stat.mutex, NULL);

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
	perror(PROGNAME ": accept");
	close(gs);
	return ACCEPT_ERROR;
      }

      if ((skkconn = calloc(1, sizeof(SkkConnection))) == NULL) {
	err_message_fnc("No enough memory.\n");
	return MEMORY_ERROR;
      }

      splen = sizeof(sp);
      if ((getpeername(s, &sp, &splen)) < 0) {
	perror(PROGNAME ": getpeername");
	memset(&sp, 0, sizeof(sp));
      }
      ipbuf[0] = '\0';
#ifdef HAVE_GETNAMEINFO
      if ((gaierr = getnameinfo(&sp, splen, ipbuf, sizeof(ipbuf),
				NULL, 0, NI_NUMERICHOST))) {
	err_message_fnc("getnameinfo(): %s\n", gai_strerror(gaierr));
	skkconn->peername = strdup("UNKNOWN");
      } else {
	ipbuf[sizeof(ipbuf) - 1] = '\0';
	skkconn->peername = strdup(ipbuf);
      }
#else
      {
	struct sockaddr_in *sp_v4 = (struct sockaddr_in *)&sp;
	skkconn->peername = strdup(inet_ntoa(sp_v4->sin_addr));
      }
#endif
      skkconn->serverstring = serverstring;
      skkconn->stat = &stat;
      skkconn->dic_list = dic_list;
      skkconn->close_after_use = 1;
      skkconn->in = s;
      skkconn->out = s;
      (void)create_server_thread(skkconn, 1);
    }
  } else {
    void *ret;
    pthread_t thread;

    if ((skkconn = calloc(1, sizeof(SkkConnection))) == NULL) {
      err_message_fnc("No enough memory.\n");
      return MEMORY_ERROR;
    }

    skkconn->peername = strdup("localhost");
    skkconn->serverstring = (char *)"localhost:127.0.0.1: ";
    /* skkconn->stat = NULL; */
    skkconn->dic_list = dic_list;
    skkconn->close_after_use = 0;
    skkconn->in = 0;
    skkconn->out = 1;
    thread = create_server_thread(skkconn, 0);
    pthread_join(thread, &ret);
  }

  return 0;
}
