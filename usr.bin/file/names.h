/*
 * Names.h - names and types used by ascmagic in file(1).
 * These tokens are here because they can appear anywhere in
 * the first HOWMANY bytes, while tokens in /etc/magic must
 * appear at fixed offsets into the file. Don't make HOWMANY
 * too high unless you have a very fast CPU.
 *
 * Copyright (c) Ian F. Darwin, 1987.
 * Written by Ian F. Darwin.
 *
 * See LEGAL.NOTICE
 *
 * $FreeBSD: src/usr.bin/file/names.h,v 1.6 1999/08/28 01:01:00 peter Exp $
 */

/* these types are used to index the table 'types': keep em in sync! */
#define L_C	0		/* first and foremost on UNIX */
#define L_CC	1		/* Bjarne's postincrement */
#define	L_FORT	2		/* the oldest one */
#define L_MAKE	3		/* Makefiles */
#define L_PLI	4		/* PL/1 */
#define L_MACH	5		/* some kinda assembler */
#define L_ENG	6		/* English */
#define	L_PAS	7		/* Pascal */
#define	L_MAIL	8		/* Electronic mail */
#define	L_NEWS	9		/* Usenet Netnews */

static char *types[] = {
	"C program text",
	"C++ program text",
	"FORTRAN program text",
	"make commands text" ,
	"PL/1 program text",
	"assembler program text",
	"English text",
	"Pascal program text",
	"mail text",
	"news text",
	"can't happen error on names.h/types",
	0};

static struct names {
	char *name;
	short type;
} names[] = {
	/* These must be sorted by eye for optimal hit rate */
	/* Add to this list only after substantial meditation */
	{"//",		L_CC},
	{"template",	L_CC},
	{"virtual",	L_CC},
	{"class",	L_CC},
	{"public:",	L_CC},
	{"private:",	L_CC},
	{"/*",		L_C},	/* must precede "The", "the", etc. */
	{"#include",	L_C},
	{"char",	L_C},
	{"The",		L_ENG},
	{"the",		L_ENG},
	{"double",	L_C},
	{"extern",	L_C},
	{"float",	L_C},
	{"real",	L_C},
	{"struct",	L_C},
	{"union",	L_C},
	{"CFLAGS",	L_MAKE},
	{"LDFLAGS",	L_MAKE},
	{"all:",	L_MAKE},
	{".PRECIOUS",	L_MAKE},
/* Too many files of text have these words in them.  Find another way
 * to recognize Fortrash.
 */
#ifdef	NOTDEF
	{"subroutine",	L_FORT},
	{"function",	L_FORT},
	{"block",	L_FORT},
	{"common",	L_FORT},
	{"dimension",	L_FORT},
	{"integer",	L_FORT},
	{"data",	L_FORT},
#endif	/*NOTDEF*/
	{".ascii",	L_MACH},
	{".asciiz",	L_MACH},
	{".byte",	L_MACH},
	{".even",	L_MACH},
	{".globl",	L_MACH},
	{".text",	L_MACH},
	{"clr",		L_MACH},
	{"(input,",	L_PAS},
	{"dcl",		L_PLI},
	{"Received:",	L_MAIL},
	{">From",	L_MAIL},
	{"Return-Path:",L_MAIL},
	{"Cc:",		L_MAIL},
	{"Newsgroups:",	L_NEWS},
	{"Path:",	L_NEWS},
	{"Organization:",L_NEWS},
	{NULL,		0}
};
#define NNAMES ((sizeof(names)/sizeof(struct names)) - 1)
