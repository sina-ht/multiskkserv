#!/bin/sh
for i in SKK-JISYO.*; do
  if ! echo $i | grep "\.cdb$" > /dev/null; then
    echo -n "Converting $i: "
    skkdic-p2cdb $i.cdb < $i;
    echo "OK."
  fi
done
