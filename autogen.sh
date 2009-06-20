#! /bin/sh

aclocal
autoheader
automake --gnu -a -c
autoconf

./configure $*
