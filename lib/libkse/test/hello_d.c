/****************************************************************************
 *
 * Simple diff mode test.
 *
 * $FreeBSD: src/lib/libkse/test/hello_d.c,v 1.2.10.1.2.1 2009/10/25 01:10:29 kensmith Exp $
 *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <pthread.h>

void *
entry(void * a_arg)
{
	fprintf(stderr, "Hello world\n");

	return NULL;
}

int
main()
{
	pthread_t thread;
	int error;

	error = pthread_create(&thread, NULL, entry, NULL);
	if (error)
		fprintf(stderr, "Error in pthread_create(): %s\n",
			strerror(error));

	error = pthread_join(thread, NULL);
	if (error)
		fprintf(stderr, "Error in pthread_join(): %s\n",
			strerror(error));

	return 0;
}
