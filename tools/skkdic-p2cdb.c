/*
 * skkdic-p2cdb.c -- convert plain skkdic to cdb.
 * (C)Copyright 2001-2010 by Hiroshi Takekawa
 * This file is part of multiskkserv.
 *
 * Last Modified: Tue Jan 26 22:36:04 2010.
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define REQUIRE_UNISTD_H
#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#if !defined(USE_TINYCDB)
#include <cdb_make.h>
#endif
#include <cdb.h>

#include "src/multiskkserv.h"
#define LINE_SIZE (SKKSERV_WORD_SIZE + SKKSERV_RESULT_SIZE)

int
main(int argc, char **argv)
{
  struct cdb_make cdb;
  int fd;
  char *sep;
  char buf[LINE_SIZE];
  char wbuf[SKKSERV_WORD_SIZE];
  char rbuf[SKKSERV_RESULT_SIZE];

  if (argc != 2) {
    printf("skkdic-p2cdb version " VERSION "\n");
    printf("(C)Copyright 2001-2010 by Hiroshi Takekawa\n\n");
    printf("usage: %s outfile < infile\n", argv[0]);
    return 1;
  }

  if (unlink(argv[1])) {
    if (errno != ENOENT) {
      perror(argv[0]);
      return 12;
    }
  }

  if ((fd = open(argv[1], O_CREAT | O_WRONLY | O_EXCL, S_IRUSR)) == -1) {
    err_message("Cannot open %s\n", argv[1]);
    return 2;
  }

  if (cdb_make_start(&cdb, fd) == -1) {
    err_message("cdb_make_start() failed.\n");
    return 3;
  }

  while (fgets(buf, LINE_SIZE, stdin)) {
    if (buf[0] == ';')
      continue;
    if (buf[0] == '\n')
      continue;

    if ((sep = strchr(buf, ' ')) == NULL) {
      err_message("format error: %s\n", buf);
      return 4;
    }
    memcpy(wbuf, buf, sep - buf);
    wbuf[sep - buf] = '\0';

    if (strlen(sep + 1) > SKKSERV_RESULT_SIZE) {
      err_message("too long entry, increase SKKSERV_RESULT_SIZE (%d -> %ld): %s\n", SKKSERV_RESULT_SIZE, strlen(sep + 1), wbuf);
      return 11;
    }
    strcpy(rbuf, sep + 1);
    /* chomp */
    if (rbuf[strlen(rbuf) - 1] == '\n')
      rbuf[strlen(rbuf) - 1] = '\0';

    if (cdb_make_add(&cdb, wbuf, strlen(wbuf), rbuf, strlen(rbuf)) == -1) {
      err_message("cdb_make_add() failed.\n");
      return 8;
    }
  }

  if (cdb_make_finish(&cdb) == -1) {
    err_message("cdb_make_finish() failed.\n");
    return 9;
  }
  if (close(fd) == -1) {
    err_message("final close() failed...\n");
    return 10;
  }

  return 0;
}
