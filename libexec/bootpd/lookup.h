/* lookup.h */
/* $FreeBSD: src/libexec/bootpd/lookup.h,v 1.2.36.1.2.1 2009/10/25 01:10:29 kensmith Exp $ */

#include "bptypes.h"	/* for int32, u_int32 */

extern u_char *lookup_hwa(char *hostname, int htype);
extern int lookup_ipa(char *hostname, u_int32 *addr);
extern int lookup_netmask(u_int32 addr, u_int32 *mask);
