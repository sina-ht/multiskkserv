## Process this file with automake to produce Makefile.in
# Makefile.am for multiskkserv
# $Id$

INCLUDES = @CDB_DIR@ -I$(top_srcdir) -I$(top_srcdir)/lib
EXTRA_DIST = getaddrinfo.c getaddrinfo.h

sbin_PROGRAMS = multiskkserv
if HAVE_GETADDRINFO
multiskkserv_SOURCES = \
	multiskkserv.c
else
multiskkserv_SOURCES = \
	multiskkserv.c getaddrinfo.c
endif

multiskkserv_LDADD = \
	$(top_builddir)/lib/libutils.la @LIBADD_PTHREAD@ @LIBADD_CDB@
