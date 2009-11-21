/*
 * This program tests if a new thread's initial tls data
 * is clean.
 *
 * David Xu <davidxu@freebsd.org>
 *
 * $FreeBSD: src/tools/regression/tls/ttls4/ttls4.c,v 1.2.22.1.2.1 2009/10/25 01:10:29 kensmith Exp $
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int __thread n;

void *f1(void *arg)
{
	if (n != 0) {
		printf("bug, n == %d \n", n);
		exit(1);
	}
	n = 1;
	return (0);
}

int main()
{
	pthread_t td;
	int i;

	for (i = 0; i < 1000; ++i) {
		pthread_create(&td, NULL, f1, NULL);
		pthread_join(td, NULL);
	}
	sleep(2);
	for (i = 0; i < 1000; ++i) {
		pthread_create(&td, NULL, f1, NULL);
		pthread_join(td, NULL);
	}
	return (0);
}
