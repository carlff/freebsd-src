#!/bin/sh
# $FreeBSD: src/tools/regression/usr.bin/pkill/pkill-x.t,v 1.1.22.1.2.1 2009/10/25 01:10:29 kensmith Exp $

base=`basename $0`

echo "1..4"

name="pkill -x"
sleep=`mktemp /tmp/$base.XXXXXX` || exit 1
ln -sf /bin/sleep $sleep
$sleep 5 &
sleep 0.3
pkill -x slee -P $$
if [ $? -ne 0 ]; then
	echo "ok 1 - $name"
else
	echo "not ok 1 - $name"
fi
pkill -x sleep -P $$
if [ $? -eq 0 ]; then
	echo "ok 2 - $name"
else
	echo "not ok 2 - $name"
fi
rm -f $sleep

name="pkill -x -f"
sleep=`mktemp /tmp/$base.XXXXXX` || exit 1
ln -sf /bin/sleep $sleep
$sleep 5 &
sleep 0.3
pkill -x -f "$sleep " -P $$
if [ $? -ne 0 ]; then
	echo "ok 3 - $name"
else
	echo "not ok 3 - $name"
fi
pkill -x -f "$sleep 5" -P $$
if [ $? -eq 0 ]; then
	echo "ok 4 - $name"
else
	echo "not ok 4 - $name"
fi
rm -f $sleep
