#!/bin/sh
# $FreeBSD: src/tools/regression/usr.bin/jot/regress.t,v 1.1.22.1.2.1 2009/10/25 01:10:29 kensmith Exp $

cd `dirname $0`

m4 ../regress.m4 regress.sh | sh
