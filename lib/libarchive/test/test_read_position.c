/*-
 * Copyright (c) 2003-2007 Tim Kientzle
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "test.h"
__FBSDID("$FreeBSD: src/lib/libarchive/test/test_read_position.c,v 1.5.2.1.2.1 2009/10/25 01:10:29 kensmith Exp $");

static unsigned char nulls[10000];
static unsigned char  buff[10000000];

/* Check that header_position tracks correctly on read. */
DEFINE_TEST(test_read_position)
{
	struct archive *a;
	struct archive_entry *ae;
	size_t write_pos;
	intmax_t read_position;
	size_t i, j;
	size_t data_sizes[] = {0, 5, 511, 512, 513};

	/* Sanity test */
	assert(sizeof(nulls) + 512 + 1024 <= sizeof(buff));

	/* Create an archive. */
	assert(NULL != (a = archive_write_new()));
	assertA(0 == archive_write_set_format_pax_restricted(a));
	assertA(0 == archive_write_set_bytes_per_block(a, 512));
	assertA(0 == archive_write_open_memory(a, buff, sizeof(buff), &write_pos));

	for (i = 0; i < sizeof(data_sizes)/sizeof(data_sizes[0]); ++i) {
		/* Create a simple archive_entry. */
		assert((ae = archive_entry_new()) != NULL);
		archive_entry_set_pathname(ae, "testfile");
		archive_entry_set_mode(ae, S_IFREG);
		archive_entry_set_size(ae, data_sizes[i]);
		assertA(0 == archive_write_header(a, ae));
		archive_entry_free(ae);
		assertA(data_sizes[i]
		    == (size_t)archive_write_data(a, nulls, sizeof(nulls)));
	}
	assertA(0 == archive_write_close(a));
	assertA(0 == archive_write_finish(a));

	/* Read the archive back. */
	assert(NULL != (a = archive_read_new()));
	assertA(0 == archive_read_support_format_tar(a));
	assertA(0 == archive_read_open_memory2(a, buff, sizeof(buff), 512));

	read_position = 0;
	/* Initial header position is zero. */
	assert(read_position == (intmax_t)archive_read_header_position(a));
	for (j = 0; j < i; ++j) {
		assertA(0 == archive_read_next_header(a, &ae));
		assert(read_position
		    == (intmax_t)archive_read_header_position(a));
		/* Every other entry: read, then skip */
		if (j & 1)
			assertEqualInt(ARCHIVE_OK,
			    archive_read_data_into_buffer(a, buff, 1));
		assertA(0 == archive_read_data_skip(a));
		/* read_data_skip() doesn't change header_position */
		assert(read_position
		    == (intmax_t)archive_read_header_position(a));

		read_position += 512; /* Size of header. */
		read_position += (data_sizes[j] + 511) & ~511;
	}

	assertA(1 == archive_read_next_header(a, &ae));
	assert(read_position == (intmax_t)archive_read_header_position(a));
	assertA(0 == archive_read_close(a));
	assert(read_position == (intmax_t)archive_read_header_position(a));
	archive_read_finish(a);
}
