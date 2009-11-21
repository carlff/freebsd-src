#!/bin/sh
#
# $FreeBSD: src/release/scripts/info-make.sh,v 1.4.36.1.2.1 2009/10/25 01:10:29 kensmith Exp $
#

# Move the info files out to their own dist
if [ -d ${RD}/trees/base/usr/share/info ]; then
	tar -cf - -C ${RD}/trees/base/usr/share/info . |
		tar -xpf - -C ${RD}/trees/info/usr/share/info &&
	rm -rf ${RD}/trees/base/usr/share/info;
fi
