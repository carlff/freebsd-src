/*	$FreeBSD: src/contrib/ipfilter/ipsend/dltest.h,v 1.2.22.1.2.1 2009/10/25 01:10:29 kensmith Exp $	*/

/*
 * Common DLPI Test Suite header file
 *
 */

/*
 * Maximum control/data buffer size (in long's !!) for getmsg().
 */
#define		MAXDLBUF	8192

/*
 * Maximum number of seconds we'll wait for any
 * particular DLPI acknowledgment from the provider
 * after issuing a request.
 */
#define		MAXWAIT		15

/*
 * Maximum address buffer length.
 */
#define		MAXDLADDR	1024


/*
 * Handy macro.
 */
#define		OFFADDR(s, n)	(u_char*)((char*)(s) + (int)(n))

/*
 * externs go here
 */
extern	void	sigalrm();
