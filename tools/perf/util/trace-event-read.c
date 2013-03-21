/*
 * Copyright (C) 2009, Steven Rostedt <srostedt@redhat.com>
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License (not later!)
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "../perf.h"
#include "util.h"
#include "trace-event.h"

static int input_fd;

int file_bigendian;
int host_bigendian;
static int long_size;

static ssize_t calc_data_size;
static bool repipe;

static int do_read(int fd, void *buf, int size)
{
	int rsize = size;

	while (size) {
		int ret = read(fd, buf, size);

		if (ret <= 0)
			return -1;

		if (repipe) {
			int retw = write(STDOUT_FILENO, buf, ret);

			if (retw <= 0 || retw != ret)
				die("repiping input file");
		}

		size -= ret;
		buf += ret;
	}

	return rsize;
}

static int read_or_die(void *data, int size)
{
	int r;

	r = do_read(input_fd, data, size);
	if (r <= 0)
		die("reading input file (size expected=%d received=%d)",
		    size, r);

	if (calc_data_size)
		calc_data_size += r;

	return r;
}

/* If it fails, the next read will report it */
static void skip(int size)
{
	char buf[BUFSIZ];
	int r;

	while (size) {
		r = size > BUFSIZ ? BUFSIZ : size;
		read_or_die(buf, r);
		size -= r;
	};
}

static unsigned int read4(struct pevent *pevent)
{
	unsigned int data;

	read_or_die(&data, 4);
	return __data2host4(pevent, data);
}

static unsigned long long read8(struct pevent *pevent)
{
	unsigned long long data;

	read_or_die(&data, 8);
	return __data2host8(pevent, data);
}

static char *read_string(void)
{
	char buf[BUFSIZ];
	char *str = NULL;
	int size = 0;
	off_t r;
	char c;

	for (;;) {
		r = read(input_fd, &c, 1);
		if (r < 0)
			die("reading input file");

		if (!r)
			die("no data");

		if (repipe) {
			int retw = write(STDOUT_FILENO, &c, 1);

			if (retw <= 0 || retw != r)
				die("repiping input file string");
		}

		buf[size++] = c;

		if (!c)
			break;
	}

	if (calc_data_size)
		calc_data_size += size;

	str = malloc(size);
	if (str)
		memcpy(str, buf, size);

	return str;
}

static int read_proc_kallsyms(struct pevent *pevent)
{
	unsigned int size;
	char *buf;

	size = read4(pevent);
	if (!size)
		return 0;

	buf = malloc(size + 1);
	if (buf == NULL)
		return -1;

	read_or_die(buf, size);
	buf[size] = '\0';

	parse_proc_kallsyms(pevent, buf, size);

	free(buf);
	return 0;
}

static int read_ftrace_printk(struct pevent *pevent)
{
	unsigned int size;
	char *buf;

	size = read4(pevent);
	if (!size)
		return 0;

	buf = malloc(size);
	if (buf == NULL)
		return -1;

	read_or_die(buf, size);

	parse_ftrace_printk(pevent, buf, size);

	free(buf);
	return 0;
}

static int read_header_files(struct pevent *pevent)
{
	unsigned long long size;
	char *header_event;
	char buf[BUFSIZ];

	read_or_die(buf, 12);

	if (memcmp(buf, "header_page", 12) != 0)
		die("did not read header page");

	size = read8(pevent);
	skip(size);

	/*
	 * The size field in the page is of type long,
	 * use that instead, since it represents the kernel.
	 */
	long_size = header_page_size_size;

	read_or_die(buf, 13);
	if (memcmp(buf, "header_event", 13) != 0)
		die("did not read header event");

	size = read8(pevent);
	header_event = malloc(size);
	if (header_event == NULL)
		return -1;

	read_or_die(header_event, size);
	free(header_event);
	return 0;
}

static int read_ftrace_file(struct pevent *pevent, unsigned long long size)
{
	char *buf;

	buf = malloc(size);
	if (buf == NULL)
		return -1;

	read_or_die(buf, size);
	parse_ftrace_file(pevent, buf, size);
	free(buf);
	return 0;
}

static int read_event_file(struct pevent *pevent, char *sys,
			    unsigned long long size)
{
	char *buf;

	buf = malloc(size);
	if (buf == NULL)
		return -1;

	read_or_die(buf, size);
	parse_event_file(pevent, buf, size, sys);
	free(buf);
	return 0;
}

static int read_ftrace_files(struct pevent *pevent)
{
	unsigned long long size;
	int count;
	int i;
	int ret;

	count = read4(pevent);

	for (i = 0; i < count; i++) {
		size = read8(pevent);
		ret = read_ftrace_file(pevent, size);
		if (ret)
			return ret;
	}
	return 0;
}

static int read_event_files(struct pevent *pevent)
{
	unsigned long long size;
	char *sys;
	int systems;
	int count;
	int i,x;
	int ret;

	systems = read4(pevent);

	for (i = 0; i < systems; i++) {
		sys = read_string();
		if (sys == NULL)
			return -1;

		count = read4(pevent);
		for (x=0; x < count; x++) {
			size = read8(pevent);
			ret = read_event_file(pevent, sys, size);
			if (ret)
				return ret;
		}
	}
	return 0;
}

ssize_t trace_report(int fd, struct pevent **ppevent, bool __repipe)
{
	char buf[BUFSIZ];
	char test[] = { 23, 8, 68 };
	char *version;
	int show_version = 0;
	int show_funcs = 0;
	int show_printk = 0;
	ssize_t size = -1;
	struct pevent *pevent;
	int err;

	*ppevent = NULL;

	calc_data_size = 1;
	repipe = __repipe;

	input_fd = fd;

	read_or_die(buf, 3);
	if (memcmp(buf, test, 3) != 0)
		die("no trace data in the file");

	read_or_die(buf, 7);
	if (memcmp(buf, "tracing", 7) != 0)
		die("not a trace file (missing 'tracing' tag)");

	version = read_string();
	if (version == NULL)
		return -1;
	if (show_version)
		printf("version = %s\n", version);
	free(version);

	read_or_die(buf, 1);
	file_bigendian = buf[0];
	host_bigendian = bigendian();

	pevent = read_trace_init(file_bigendian, host_bigendian);
	if (pevent == NULL) {
		pr_debug("read_trace_init failed");
		goto out;
	}

	read_or_die(buf, 1);
	long_size = buf[0];

	page_size = read4(pevent);

	err = read_header_files(pevent);
	if (err)
		goto out;
	err = read_ftrace_files(pevent);
	if (err)
		goto out;
	err = read_event_files(pevent);
	if (err)
		goto out;
	err = read_proc_kallsyms(pevent);
	if (err)
		goto out;
	err = read_ftrace_printk(pevent);
	if (err)
		goto out;

	size = calc_data_size - 1;
	calc_data_size = 0;
	repipe = false;

	if (show_funcs) {
		pevent_print_funcs(pevent);
	} else if (show_printk) {
		pevent_print_printk(pevent);
	}

	*ppevent = pevent;
	pevent = NULL;

out:
	if (pevent)
		pevent_free(pevent);
	return size;
}
