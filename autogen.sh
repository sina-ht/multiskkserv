#! /bin/sh

aclocal
autoheader
automake --gnu -a -c Makefile
for i in `find . -name Makefile.am`; do
  f=`echo $i | sed 's/.am//'`
  automake $f
done
autoconf

./configure $*
