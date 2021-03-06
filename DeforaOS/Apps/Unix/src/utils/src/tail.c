/* $Id$ */
/* Copyright (c) 2006-2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix utils */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>


/* tail */
/* types */
typedef struct _Prefs
{
	int flags;
	int bytes;
	int lines;
} Prefs;
#define TAIL_PREFS_f	0x1

static int _tail_error(char const * message, int ret);
static int _tail_do_bytes(Prefs * prefs, FILE * fp, char const * filename);
static int _tail_do_lines(Prefs * prefs, FILE * fp, char const * filename);
static int _tail_wait(FILE * fp, char const * filename);

static int _tail(Prefs * prefs, char const * filename)
{
	int ret;
	FILE * fp = stdin;

	if(filename != NULL && (fp = fopen(filename, "r")) == NULL)
		return _tail_error(filename, 1);
	if(prefs->bytes != 0)
		ret = _tail_do_bytes(prefs, fp, filename != NULL
				? filename : "stdin");
	else
		ret = _tail_do_lines(prefs, fp, filename != NULL
				? filename : "stdin");
	if(prefs->flags & TAIL_PREFS_f)
		_tail_wait(fp, filename);
	if(filename != NULL && fclose(fp) != 0)
		return _tail_error(filename, 1);
	return ret;
}

static int _tail_error(char const * message, int ret)
{
	fputs("tail: ", stderr);
	perror(message);
	return ret;
}

static int _tail_do_bytes(Prefs * prefs, FILE * fp, char const * filename)
{
	char buf[256];
	size_t i;
	long c = prefs->bytes;

	if(fseek(fp, c > 0 ? c - 1 : c, c > 0 ? SEEK_SET : SEEK_END) != 0)
	{
		/* FIXME implement */
		fprintf(stderr, "tail: %s\n", strerror(ENOSYS));
		return 1;
	}
	while((i = fread(buf, 1, sizeof(buf), fp)) > 0)
		if(fwrite(buf, 1, i, stdout) != i)
			return _tail_error("stdout", 1);
	if(!feof(fp))
		return _tail_error(filename, 1);
	return 0;
}

/* tail_do_lines */
static void _lines_rotate(char ** lines, int pos);
static int _lines_realloc(char ** lines, int pos, size_t column);
static void _lines_print(char ** lines, int pos);

static int _tail_do_lines(Prefs * prefs, FILE * fp, char const * filename)
{
	int ret = 0;
	char ** lines;
	int c;
	int pos = 0;
	size_t column = 0;

	if(prefs->lines == 0)
		return 0;
	if((lines = calloc(sizeof(*lines), prefs->lines)) == NULL)
		return _tail_error(filename, 1);
	while((c = fgetc(fp)) != EOF)
	{
		if(column == 0 && lines[pos] != NULL)
		{
			if(pos + 1 < prefs->lines)
				pos++;
			else
				_lines_rotate(lines, pos);
		}
		if(_lines_realloc(lines, pos, column) != 0)
			break;
		if(c != '\n')
		{
			lines[pos][column++] = c;
			continue;
		}
		lines[pos][column] = '\0';
		column = 0;
	}
	if(lines[0] != NULL)
		_lines_print(lines, pos);
	if(c != EOF || !feof(fp))
		ret = _tail_error(filename, 1);
	free(lines);
	return ret;
}

static void _lines_rotate(char ** lines, int pos)
{
	int i;

	free(lines[0]);
	for(i = 1; i <= pos; i++)
		lines[i - 1] = lines[i];
	lines[pos] = NULL;
}

static int _lines_realloc(char ** lines, int pos, size_t column)
{
	char * p;

	if((p = realloc(lines[pos], column + 1)) == NULL)
		return 1;
	lines[pos] = p;
	return 0;
}

static void _lines_print(char ** lines, int pos)
{
	int i;

	for(i = 0; i <= pos; i++)
	{
		puts(lines[i]);
		free(lines[i]);
	}
}

static int _tail_wait(FILE * fp, char const * filename)
{
	int fd = fileno(fp);
	struct stat st1;
	struct stat st2;
	off_t seek;
	size_t size;
	size_t i;
	char buf[1024];
	ssize_t ssize;

	if(fstat(fd, &st1) != 0)
		return -_tail_error(filename, 1);
	for(;;)
	{
		sleep(1);
		if(fstat(fd, &st2) != 0)
			return -_tail_error(filename, 1);
		if(st1.st_dev != st2.st_dev || st1.st_ino != st2.st_ino)
		{
			/* XXX this code path is probably impossible */
			seek = 0;
			size = st2.st_size;
		}
		else if(st2.st_size > st1.st_size)
		{
			seek = st1.st_size;
			size = st2.st_size - st1.st_size;
		}
		else
			continue;
		fseek(fp, seek, SEEK_SET);
		for(i = 0; i < size; i+=sizeof(buf))
			if((ssize = fread(buf, sizeof(*buf), sizeof(buf), fp))
					> 0)
				fwrite(buf, sizeof(*buf), ssize, stdout);
		st1 = st2;
	}
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: tail [-f][-c number|-n number][file]\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	Prefs prefs;
	char * p;

	memset(&prefs, 0, sizeof(prefs));
	prefs.lines = 10;
	while((o = getopt(argc, argv, "fc:n:")) != -1)
		switch(o)
		{
			case 'f':
				prefs.flags |= TAIL_PREFS_f;
				break;
			case 'c':
				prefs.lines = 0;
				prefs.bytes = strtol(optarg, &p, 10);
				if(optarg[0] == '\0' || *p != '\0')
					return _usage();
				break;
			case 'n':
				prefs.bytes = 0;
				prefs.lines = strtol(optarg, &p, 10);
				if(optarg[0] == '\0' || *p != '\0')
					return _usage();
				break;
			default:
				return _usage();
		}
	if(optind != argc && optind + 1 != argc)
		return _usage();
	return (_tail(&prefs, argv[optind]) == 0) ? 0 : 2;
}
