## Process this file with automake to produce Makefile.in
# Makefile.am for multiskkserv
# $Id$

INCLUDES = @CDB_INCLUDES@ -I$(top_srcdir) -I$(top_srcdir)/lib
EXTRA_DIST = getaddrinfo.c getaddrinfo.h

sbin_PROGRAMS = multiskkserv multiskkserv-ctl

if HAVE_GETADDRINFO
multiskkserv_SOURCES = \
	multiskkserv.c getopt-support.c
multiskkserv_ctl_SOURCES = \
	multiskkserv-ctl.c getopt-support.c
else
multiskkserv_SOURCES = \
	multiskkserv.c getopt-support.c getaddrinfo.c
multiskkserv_ctl_SOURCES = \
	multiskkserv-ctl.c getopt-support.c getaddrinfo.c
endif

multiskkserv_LDADD = \
	$(top_builddir)/lib/libutils.la @LIBADD_PTHREAD@ @LIBADD_CDB@
multiskkserv_ctl_LDADD = \
	$(top_builddir)/lib/libutils.la
