multiskkserv -- simple skk multi-dictionary server
(C)Copyright 2001 by Hiroshi Takekawa <sian@big.or.jp>
Last Modified: Tue Feb 13 11:14:49 2001.
$Id$

multiskkservは複数の辞書を扱える辞書サーバです。multiskkservは辞書の
フォーマットとして、Dan  J. Bernsteinによる固定データベースcdbを採用し
ています。このcdbは、システムにインストールされた固定辞書を保持するの
に向いていると思われます。

このソフトウェアは試験的なものです。バグレポートやコメントなどをお願いします。
少なくとも、このREADMEはmultiskkservを辞書サーバとして使って書けています。

1. 必要な環境

pthread:
 もしlibc5を使っていれば、linuxthreadをいれてください。
 http://pauillac.inria.fr/~xleroy/linuxthreads/
 あたりでみつかるでしょう。

cdb-0.75:
 cdbのパッケージをインストールはしなくてもいいので、コンパイルだけして
 ください。インストールすればきっと役に立つとは思いますが。
 http://cr.yp.to/cdb/cdb-0.75.tar.gz を入手して、
 展開し、その中に移動して、'make it'を実行してください。
 'make setup check'としてインストールすることもできます。

2. コンパイルおよびインストール

--with-cdb で cdb を展開しコンパイルしたディレクトリをフルパスで指定し
てください。

% tar xvzf multiskkserv-2001xxxx.tar.gz
% mkdir multiskkserv.build && cd multiskkserv.build
% ../multiskkserv-2001xxxx/configure --with-cdb=/usr/src/cdb-0.75 && make
% su
# make install
# exit
% cd ..
% rm -rf multiskkserv.build

3. 辞書の変換

'make install'すると、2つのバイナリがインストールされます。
multiskkservがsbinに、skkdic-p2cdbがbinに入ります。skkdic-p2cdbを使ってプレーンテキストの辞書をcdb形式に変換します。やり方は簡単です。

% cd /usr/local/share/skk
% skkdic-p2cdb SKK-JISYO.L.cdb < SKK-JISYO.L.cdb

4. サーバの起動

サーバの起動も簡単です。

# /usr/local/sbin/multiskkserv /usr/local/share/skk/SKK-JISYO.L.cdb &

multiskkservは自動でdetachしないので&をつける必要があります。
複数の辞書を指定することもできます。

# /usr/local/sbin/multiskkserv /usr/local/share/skk/SKK-JISYO.L.cdb /usr/local/share/skk/SKK-JISYO.zipcode.cdb &

-nを使えばinetdやtcpserverからもおそらく使えるはずです。(未確認)

inetd.conf:
skkserv stream  tcp     nowait  nobody  /usr/sbin/tcpd /usr/local/sbin/multiskkserv -n /usr/local/share/skk/SKK-JISYO.L.cdb

tcpserver:
tcpserver -v -R 0 skkserv /usr/local/sbin/multiskkserv -n /usr/local/share/skk/SKK-JISYO.L.cdb

他のオプションについては、

% /usr/local/sbin/multiskkserv -h

とすると表示されます。

5. ライセンス

GPL version 2に従います。詳細はCOPYING(参考訳 COPYING.j)を読んでください。

6. 連絡先

E-mail: Hiroshi Takekawa <sian@big.or.jp>
