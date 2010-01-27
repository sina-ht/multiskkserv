## Process this file with automake to produce Makefile.in
# Makefile.am for multiskkserv

SUBDIRS = lib src tools
EXTRA_DIST = COPYING.j README.j common.h compat.h test/SKK-JISYO.S test/henkan-euc.txt test/henkandekinai-euc.txt test/ka-euc.txt test/kan-euc.txt test/kanji-euc.txt

check:
	@mkdir -p $(top_builddir)/test
	$(top_builddir)/tools/skkdic-p2cdb $(top_builddir)/test/SKK-JISYO.S.cdb < $(top_srcdir)/test/SKK-JISYO.S
	$(top_builddir)/src/multiskkserv -s 127.0.0.1 -p 12347 $(top_builddir)/test/SKK-JISYO.S.cdb &
	@sleep 1
	@$(top_builddir)/src/multiskkserv-ctl -s 127.0.0.1 -p 12347 stat
	$(top_builddir)/src/multiskkserv-ctl -s 127.0.0.1 -p 12347 -y `cat $(top_srcdir)/test/kan-euc.txt` tran > $(top_builddir)/test/kan-euc-tran.txt
	$(top_builddir)/src/multiskkserv-ctl -s 127.0.0.1 -p 12347 -y `cat $(top_srcdir)/test/ka-euc.txt` tran > $(top_builddir)/test/ka-euc-tran.txt
	$(top_builddir)/src/multiskkserv-ctl -s 127.0.0.1 -p 12347 -y `cat $(top_srcdir)/test/kanji-euc.txt` tran > $(top_builddir)/test/kanji-euc-tran.txt
	$(top_builddir)/src/multiskkserv-ctl -s 127.0.0.1 -p 12347 -y `cat $(top_srcdir)/test/henkan-euc.txt` tran > $(top_builddir)/test/henkan-euc-tran.txt
	$(top_builddir)/src/multiskkserv-ctl -s 127.0.0.1 -p 12347 -y `cat $(top_srcdir)/test/henkandekinai-euc.txt` tran > $(top_builddir)/test/henkandekinai-euc-tran.txt
	killall src/multiskkserv
	@nkf -w $(top_srcdir)/test/kan-euc.txt
	@nkf -w $(top_builddir)/test/kan-euc-tran.txt
	@nkf -w $(top_srcdir)/test/ka-euc.txt
	@nkf -w $(top_builddir)/test/ka-euc-tran.txt
	@nkf -w $(top_srcdir)/test/kanji-euc.txt
	@nkf -w $(top_builddir)/test/kanji-euc-tran.txt
	@nkf -w $(top_srcdir)/test/henkan-euc.txt
	@nkf -w $(top_builddir)/test/henkan-euc-tran.txt
	@nkf -w $(top_srcdir)/test/henkandekinai-euc.txt
	@nkf -w $(top_builddir)/test/henkandekinai-euc-tran.txt
	@echo

clean-local:
	rm -f $(top_builddir)/test/SKK-JISYO.S.cdb $(top_builddir)/test/*-tran.txt
