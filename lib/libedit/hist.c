/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Christos Zoulas of Cornell University.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$NetBSD: hist.c,v 1.15 2003/11/01 23:36:39 christos Exp $
 */

#if !defined(lint) && !defined(SCCSID)
static char sccsid[] = "@(#)hist.c	8.1 (Berkeley) 6/4/93";
#endif /* not lint && not SCCSID */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libedit/hist.c,v 1.6.10.1.2.1 2009/10/25 01:10:29 kensmith Exp $");

/*
 * hist.c: History access functions
 */
#include "sys.h"
#include <stdlib.h>
#include "el.h"

/* hist_init():
 *	Initialization function.
 */
protected int
hist_init(EditLine *el)
{

	el->el_history.fun = NULL;
	el->el_history.ref = NULL;
	el->el_history.buf = (char *) el_malloc(EL_BUFSIZ);
	el->el_history.sz  = EL_BUFSIZ;
	if (el->el_history.buf == NULL)
		return (-1);
	el->el_history.last = el->el_history.buf;
	return (0);
}


/* hist_end():
 *	clean up history;
 */
protected void
hist_end(EditLine *el)
{

	el_free((ptr_t) el->el_history.buf);
	el->el_history.buf = NULL;
}


/* hist_set():
 *	Set new history interface
 */
protected int
hist_set(EditLine *el, hist_fun_t fun, ptr_t ptr)
{

	el->el_history.ref = ptr;
	el->el_history.fun = fun;
	return (0);
}


/* hist_get():
 *	Get a history line and update it in the buffer.
 *	eventno tells us the event to get.
 */
protected el_action_t
hist_get(EditLine *el)
{
	const char *hp;
	int h;

	if (el->el_history.eventno == 0) {	/* if really the current line */
		(void) strncpy(el->el_line.buffer, el->el_history.buf,
		    el->el_history.sz);
		el->el_line.lastchar = el->el_line.buffer +
		    (el->el_history.last - el->el_history.buf);

#ifdef KSHVI
		if (el->el_map.type == MAP_VI)
			el->el_line.cursor = el->el_line.buffer;
		else
#endif /* KSHVI */
			el->el_line.cursor = el->el_line.lastchar;

		return (CC_REFRESH);
	}
	if (el->el_history.ref == NULL)
		return (CC_ERROR);

	hp = HIST_FIRST(el);

	if (hp == NULL)
		return (CC_ERROR);

	for (h = 1; h < el->el_history.eventno; h++)
		if ((hp = HIST_NEXT(el)) == NULL) {
			el->el_history.eventno = h;
			return (CC_ERROR);
		}
	(void) strlcpy(el->el_line.buffer, hp,
			(size_t)(el->el_line.limit - el->el_line.buffer));
	el->el_line.lastchar = el->el_line.buffer + strlen(el->el_line.buffer);

	if (el->el_line.lastchar > el->el_line.buffer
	    && el->el_line.lastchar[-1] == '\n')
		el->el_line.lastchar--;
	if (el->el_line.lastchar > el->el_line.buffer
	    && el->el_line.lastchar[-1] == ' ')
		el->el_line.lastchar--;
#ifdef KSHVI
	if (el->el_map.type == MAP_VI)
		el->el_line.cursor = el->el_line.buffer;
	else
#endif /* KSHVI */
		el->el_line.cursor = el->el_line.lastchar;

	return (CC_REFRESH);
}


/* hist_command()
 *	process a history command
 */
protected int
hist_command(EditLine *el, int argc, const char **argv)
{
	const char *str;
	int num;
	HistEvent ev;

	if (el->el_history.ref == NULL)
		return (-1);

	if (argc == 1 || strcmp(argv[1], "list") == 0) {
		 /* List history entries */

		for (str = HIST_LAST(el); str != NULL; str = HIST_PREV(el))
			(void) fprintf(el->el_outfile, "%d %s",
			    el->el_history.ev.num, str);
		return (0);
	}

	if (argc != 3)
		return (-1);

	num = (int)strtol(argv[2], NULL, 0);

	if (strcmp(argv[1], "size") == 0)
		return history(el->el_history.ref, &ev, H_SETSIZE, num);

	if (strcmp(argv[1], "unique") == 0)
		return history(el->el_history.ref, &ev, H_SETUNIQUE, num);

	return -1;
}

/* hist_enlargebuf()
 *	Enlarge history buffer to specified value. Called from el_enlargebufs().
 *	Return 0 for failure, 1 for success.
 */
protected int
/*ARGSUSED*/
hist_enlargebuf(EditLine *el, size_t oldsz, size_t newsz)
{
	char *newbuf;

	newbuf = realloc(el->el_history.buf, newsz);
	if (!newbuf)
		return 0;

	(void) memset(&newbuf[oldsz], '\0', newsz - oldsz);

	el->el_history.last = newbuf +
				(el->el_history.last - el->el_history.buf);
	el->el_history.buf = newbuf;
	el->el_history.sz  = newsz;

	return 1;
}
