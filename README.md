NAME
====

**dsm2sql** - read Dutch Smart Meter (DSM) telegrams from port P1 and
convert to MariaDB-INSERT queries

SYNOPSIS
========

**dsm2sql** \[**-h host**\] \[**-P port**\] \[**-u username**\] \[**-ppassword**\] \[**-c comport**\] \[**-d **\] **db\_name**

DESCRIPTION
===========

sdm2sql reads telegrams from a Dutch Smart Meter (DSM) from port P1 and
convert these to MariaDB-INSERT queries such that the data can be stored
in a MariaDB or similar database. Each telegram contains a checksum,
only telegrams with a correct checksum are output by dsm2sql.

**Warning**: the computer running dsm2sql should be equipped with a
serial port which is connect the DSM\'s port P1. In case of a Raspberry
PI a special cable is required.

A Dutch Smart Meter (DSM), is actually not smart, \"smart\" only means
the meter can be read by the utility provider. From the start the
specification of a DSM contains a port P1 which interface description is
\"open\". The interface of P1 is fully described in a requirement
specification. This implies that the precise way of working can be read
by reading the specification. Since the start the specification was
updated a number of times. All specifications can be found at on the
website of Netbeheer Nederland, \<https://www.netbeheernederland.nl/\>.

dsm2sql was written for a DSM which complies to version 4.2, as
described in: \"Dutch Smart Meter Requirements v4.2.2 Final P1 (14 March
2014)\".

OPTIONS
=======

**-h host**
     
     connect to the MariaDB server on the given host.

**-P port**

     the TCP/IP port number to use for the connection to the MariaDB host (default 3306).

**-u username**

     the MariaDB user name to use when connecting to the server.

**-ppassword**

    the password to use when connecting to the server.

**-c comport**

    RS232 port on de computer running dsm2sql. On Raspberry PI: ttyAMA0.

**-d**

    only write the MariaDB-INSERT to stdout.

**db\_name**

    name of the database.

DATABASE
========

dsm2sql assumes presence of a database with the table which will be
created by below SQL code. The name of the database can be chosen
freely.

```
$ cat createdb.sql
CREATE DATABASE IF NOT EXISTS `energie`;
DROP TABLE IF EXISTS `emeter`;
CREATE TABLE `emeter` (
  `id`        int(11) unsigned NOT NULL AUTO_INCREMENT,
  `e_dattijd` datetime DEFAULT NULL,
  `e_ver_t1`  float(9,3) DEFAULT NULL,
  `e_ver_t2`  float(9,3) DEFAULT NULL,
  `e_ter_t1`  float(9,3) DEFAULT NULL,
  `e_ter_t2`  float(9,3) DEFAULT NULL,
  `t`         tinyint(1) unsigned DEFAULT NULL,
  `e_ver_mom` float(5,3) DEFAULT NULL,
  `e_ter_mom` float(5,3) DEFAULT NULL,
  `g_dattijd` datetime DEFAULT NULL,
  `g_ver`     float(8,3) DEFAULT NULL,
   PRIMARY KEY (`id`)
  ) ENGINE=InnoDB AUTO_INCREMENT=4330 DEFAULT CHARSET=latin1;

```

EXAMPLE
=======

To read one telegram using a Raspberry PI from port P1 and view the
resulting MySQL-INSERT query:

```
$ dsm2sql -u account -c ttyAMA0 -pyrpassword db_name -d
/KFM5KAIFA-METER
1-3:0.2.8(42)
0-0:1.0.0(161106115629W)
0-0:96.1.1(idfrommymeter)
1-0:1.8.1(000350.759*kWh)
1-0:1.8.2(000159.184*kWh)
1-0:2.8.1(000358.177*kWh)
1-0:2.8.2(001041.184*kWh)
0-0:96.14.0(0001)
1-0:1.7.0(00.000*kW)
1-0:2.7.0(00.998*kW)
0-0:96.7.21(00001)
0-0:96.7.9(00001)
1-0:99.97.0(1)(0-0:96.7.19)(000101000011W)(2147483647*s)
1-0:32.32.0(00000)
1-0:52.32.0(00000)
1-0:72.32.0(00000)
1-0:32.36.0(00000)
1-0:52.36.0(00000)
1-0:72.36.0(00000)
0-0:96.13.1()
0-0:96.13.0()
1-0:31.7.0(000*A)
1-0:51.7.0(005*A)
1-0:71.7.0(002*A)
1-0:21.7.0(00.018*kW)
1-0:41.7.0(00.000*kW)
1-0:61.7.0(00.220*kW)
1-0:22.7.0(00.000*kW)
1-0:42.7.0(01.258*kW)
1-0:62.7.0(00.000*kW)
0-1:24.1.0(003)
0-1:96.1.0(idfrommymeter)
0-1:24.2.1(161106110000W)(00185.352*m3)
!7AC8
[2016-11-06 10:56:32] INSERT INTO emeter (e_dattijd,
e_ver_t1,e_ver_t2,e_ter_t1,e_ter_t2,t,
e_ver_mom,e_ter_mom,g_dattijd,g_ver) VALUES ("2016-11-06
11:56:29",000350.759,000159.184,000358.177,001041.184,0001,00.000,00.998,"2016-11-06
11:00:00",00185.352);
```

AUTHOR
======
Luc Castermans \<luc.castermans\@gmail.com\>
