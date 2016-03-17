# zundoko\_storage\_engine

* ズンドコキヨシ用MySQLストレージエンジン


# How to build

* CentOS 6.6上のMySQL 5.6.29で動作確認しています。

```
$ wget http://dev.mysql.com/get/Downloads/MySQL-5.6/mysql-5.6.29.tar.gz
$ tar xf mysql-5.6.29.tar.gz
$ cd mysql-5.6.29
$ cp -r /path/to/zundoko_storage_engine storage/
$ cmake .
$ make
$ sudo make install

mysql> INSTALL PLUGIN zundoko SONAME 'ha_zundoko.so';
```


# 実行

```
mysql> CREATE TABLE t1 (val varchar(32)) Engine= zundoko;
mysql56> SELECT * FROM t1;
+--------------------+
| val                |
+--------------------+
| ドコ               |
| ズン               |
| ズン               |
| ドコ               |
| ズン               |
| ズン               |
| ドコ               |
| ズン               |
| ドコ               |
| ズン               |
| ズン               |
| ズン               |
| ズン               |
| ズン               |
| ドコ               |
| キ・ヨ・シ！       |
+--------------------+
16 rows in set (0.00 sec)
```


# おぼえがき

* キ・ヨ・シ！ をstoreしたところでHA_ERR_END_OF_FILEを返しちゃうと、そのまま出力が終わっちゃうので、キ・ヨ・シ！ した次のrnd_nextでHA_ERR_END_OF_FILEを返さないといけない。それに気付かずにハマった。
* rnd_nextで複数行返せないこともちょっと忘れてた。
* SELECTしかしないやつならやっぱりinformation_schemaプラグインの方が圧倒的に楽。
