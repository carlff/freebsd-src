#! /bin/sh -
#
# $FreeBSD: src/gnu/usr.bin/groff/src/roff/psroff/psroff.sh,v 1.2.36.1.2.1 2009/10/25 01:10:29 kensmith Exp $

exec groff -Tps -l -C ${1+"$@"}
