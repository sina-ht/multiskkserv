multiskkserv -- simple skk multi-dictionary server
(C) Copyright 2001, 2002 by Hiroshi Takekawa <sian@big.or.jp>
Last Modified: Wed Jan 30 18:55:27 2002.
$Id$

multiskkserv は複数の辞書を扱える辞書サーバです。 multiskkserv は辞書
のフォーマットとして、 Dan J. Bernstein による固定データベース cdb を
採用しています。この cdb は、システムにインストールされた固定辞書を保
持するのに向いていると思われます。

このソフトウェアは試験的なものです。バグレポートやコメントなどをお願い
します。少なくとも、この README は multiskkserv を辞書サーバとして使っ
て書けています。


1. 必要な環境

pthread:
 もし libc5 を使っていれば、 linuxthread をいれてください。
 http://pauillac.inria.fr/~xleroy/linuxthreads/
 あたりでみつかるでしょう。

cdb-0.75:
 cdb のパッケージをインストールはしなくてもいいので、コンパイルだけし
 てください。インストールすればきっと役に立つとは思いますが。
 http://cr.yp.to/cdb/cdb-0.75.tar.gz を入手して、
 展開し、その中に移動して、'make it'を実行してください。
 'make setup check'としてインストールすることもできます。


2. コンパイルおよびインストール

--with-cdb で cdb を展開しコンパイルしたディレクトリをフルパスで指定し
てください。

% tar xvzf multiskkserv-2002xxxx.tar.gz
% mkdir multiskkserv.build && cd multiskkserv.build
% ../multiskkserv-2002xxxx/configure --with-cdb=/usr/src/cdb-0.75 && make
% su
# make install
# exit
% cd ..
% rm -rf multiskkserv.build

'make install'すると、 3 つのバイナリがインストールされます。
multiskkserv, multiskkserv-ctl が sbin に、 skkdic-p2cdb が bin に入り
ます。


3. 辞書の変換

skkdic-p2cdb を使ってプレーンテキストの辞書を cdb 形式に変換します。や
り方は簡単です。

# cd /usr/local/share/skk
# skkdic-p2cdb SKK-JISYO.L.cdb < SKK-JISYO.L


4. サーバの起動

サーバの起動も簡単です。

# /usr/local/sbin/multiskkserv /usr/local/share/skk/SKK-JISYO.L.cdb &

multiskkserv は自動で detach しないので&をつける必要があります。
複数の辞書を指定することもできます。

# /usr/local/sbin/multiskkserv /usr/local/share/skk/SKK-JISYO.L.cdb /usr/local/share/skk/SKK-JISYO.zipcode.cdb &

セキュリティが気になるという方は、一般ユーザで起動し、さらに chroot さ
せる -r をつけるといいでしょう。

% /usr/local/sbin/multiskkserv -r /usr/local/share/skk SKK-JISYO.L.cdb &

-n を使えば inetd や tcpserver からもおそらく使えるはずです。 (未確認)

inetd.conf:
skkserv stream  tcp     nowait  nobody  /usr/sbin/tcpd /usr/local/sbin/multiskkserv -n /usr/local/share/skk/SKK-JISYO.L.cdb

tcpserver:
tcpserver -v -R 0 skkserv /usr/local/sbin/multiskkserv -n /usr/local/share/skk/SKK-JISYO.L.cdb

他のオプションについては、

% /usr/local/sbin/multiskkserv -h

とすると表示されます。


5. 稼働情報の確認

% /usr/local/sbin/multiskkserv-ctl -s hostname stat

とすると、接続回数と現在接続されているクライアントの数を表示します。こ
れには skkserv の protocol の独自 extension が使われています。


6. ライセンス

GPL version 2 に従います。詳細は COPYING (参考訳 COPYING.j) を読んでください。


7. 連絡先

E-mail: Hiroshi Takekawa <sian@big.or.jp>


8. 謝辞

Eiji Obata <obata@suzuki.kuee.kyoto-u.ac.jp>さん:
20020113 に必要なヘッダファイルが含まれていないことを指摘してくださいました。
