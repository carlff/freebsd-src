#!/bin/sh
# $FreeBSD: src/tools/regression/usr.bin/pkill/pkill-_g.t,v 1.1.2.1.2.1 2009/10/25 01:10:29 kensmith Exp $

base=`basename $0`

echo "1..2"

name="pkill -G <gid>"
rgid=`id -gr`
sleep=`mktemp /tmp/$base.XXXXXX` || exit 1
ln -sf /bin/sleep $sleep
$sleep 5 &
sleep 0.3
pkill -f -G $rgid $sleep
ec=$?
case $ec in
0)
	echo "ok 1 - $name"
	;;
*)
	echo "not ok 1 - $name"
	;;
esac
rm -f $sleep

name="pkill -G <group>"
rgid=`id -grn`
sleep=`mktemp /tmp/$base.XXXXXX` || exit 1
ln -sf /bin/sleep $sleep
$sleep 5 &
sleep 0.3
pkill -f -G $rgid $sleep
ec=$?
case $ec in
0)
	echo "ok 2 - $name"
	;;
*)
	echo "not ok 2 - $name"
	;;
esac
rm -f $sleep
