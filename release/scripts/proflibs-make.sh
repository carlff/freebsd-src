#!/bin/sh
#
# $FreeBSD: src/release/scripts/proflibs-make.sh,v 1.8.30.1.2.1 2009/10/25 01:10:29 kensmith Exp $
#

# Move the profiled libraries out to their own dist
for i in ${RD}/trees/base/usr/lib/*_p.a; do
	mv $i ${RD}/trees/proflibs/usr/lib
done
