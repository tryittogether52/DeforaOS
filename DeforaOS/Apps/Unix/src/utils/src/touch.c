/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
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



#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <utime.h>
#include <errno.h>


/* Prefs */
#define TOUCH_PREFS_a 1
#define TOUCH_PREFS_m 2
#define TOUCH_PREFS_c 4
#define TOUCH_PREFS_r 8
#define TOUCH_PREFS_t 16
typedef struct _Prefs
{
	int flags;
	char * rtime;
	time_t ttime;
} Prefs;

static int _prefs_ttime(char * string, time_t * time);
static int _prefs_parse(Prefs * prefs, int argc, char * argv[])
{
	int o;

	memset(prefs, 0, sizeof(Prefs));
	while((o = getopt(argc, argv, "acmr:t:")) != -1)
		switch(o)
		{
			case 'a':
				prefs->flags |= TOUCH_PREFS_a;
				break;
			case 'c':
				prefs->flags |= TOUCH_PREFS_c;
				break;
			case 'm':
				prefs->flags |= TOUCH_PREFS_m;
				break;
			case 'r':
				prefs->flags -= prefs->flags & TOUCH_PREFS_t;
				prefs->flags |= TOUCH_PREFS_r;
				prefs->rtime = optarg;
				break;
			case 't':
				prefs->flags -= prefs->flags & TOUCH_PREFS_r;
				prefs->flags |= TOUCH_PREFS_t;
				if(!_prefs_ttime(optarg, &prefs->ttime))
					return 1;
				break;
			default:
				return 1;
		}
	if(!((prefs->flags & TOUCH_PREFS_a) && (prefs->flags & TOUCH_PREFS_m)))
		prefs->flags |= (TOUCH_PREFS_a | TOUCH_PREFS_m);
	return 0;
}

static int _ttime_century(char ** p, time_t * time);
static int _ttime_year(char ** p, time_t * time);
static int _ttime_month(char ** p, time_t * time);
static int _ttime_day(char ** p, time_t * time);
static int _ttime_hour(char ** p, time_t * time);
static int _ttime_minut(char ** p, time_t * time);
static int _ttime_second(char ** p, time_t * time);
static int _prefs_ttime(char * string, time_t * time)
{
	time_t t = 0;
	char ** p = &string;
	int ret = 0;
	int len;

	switch((len = strlen(string)))
	{
		case 15:
		case 12:
			ret += _ttime_century(p, &t);
		case 13:
		case 10:
			ret += _ttime_year(p, &t);
		case 11:
		case 8:
			ret += _ttime_month(p, &t);
			ret += _ttime_day(p, &t);
			ret += _ttime_hour(p, &t);
			ret += _ttime_minut(p, &t);
			if(len % 2)
				ret += _ttime_second(p, &t);
			break;
		default:
			return 1;
	}
	if(ret != 0)
		return 1;
	*time = t;
	return 0;
}

static int _ttime_number(char ** p, time_t * res);
static int _ttime_century(char ** p, time_t * time)
{
	/* FIXME */
	return 1;
}

static int _ttime_number(char ** p, time_t * res)
{
	if(**p >= '0' && **p <= '9')
	{
		*res = (**p - '0') * 10;
		(*p)++;
		if(**p >= '0' && **p <= '9')
		{
			*res += (**p - '0');
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %u\n", *res);
#endif
			(*p)++;
			return 0;
		}
	}
	return 1;
}

static int _ttime_year(char ** p, time_t * time)
{
	time_t res;

	if(_ttime_number(p, &res) != 0)
		return 1;
	*time = res * 60 * 60 * 24 * 365; /* FIXME */
	return 0;
}

static int _ttime_month(char ** p, time_t * time)
{
	time_t res;

	if(_ttime_number(p, &res) != 0)
		return 1;
	*time = res * 60 * 60 * 24 * 30; /* FIXME */
	return 0;
}

static int _ttime_day(char ** p, time_t * time)
{
	time_t res;

	if(_ttime_number(p, &res) != 0)
		return 1;
	*time = res * 60 * 60 * 24;
	return 0;
}

static int _ttime_hour(char ** p, time_t * time)
{
	time_t res;

	if(_ttime_number(p, &res) != 0)
		return 1;
	*time += res * 60 * 60;
	return 0;
}

static int _ttime_minut(char ** p, time_t * time)
{
	time_t res;

	if(_ttime_number(p, &res) != 0 || res >= 60)
		return 1;
	*time += res * 60;
	return 0;
}

static int _ttime_second(char ** p, time_t * time)
{
	time_t res;

	if(**p != '.')
		return 1;
	(*p)++;
	if(_ttime_number(p, &res) != 0 || res >= 60)
		return 1;
	*time += res;
	return 0;
}


/* touch */
static int _touch_error(char const * message, int ret);
static int _touch_rtime(char const * filename, time_t * atime, time_t * mtime);
static int _touch_do(Prefs * prefs, char const * filename,
		time_t atime, time_t mtime);

static int _touch(Prefs * prefs, int argc, char * argv[])
{
	int ret = 0;
	time_t atime = prefs->ttime;
	time_t mtime = prefs->ttime;
	int i;

	if(prefs->flags & TOUCH_PREFS_r)
	{
		if(_touch_rtime(prefs->rtime, &atime, &mtime) != 0)
			return 1;
	}
	else if(!(prefs->flags & TOUCH_PREFS_t))
	{
		atime = time(NULL);
		mtime = atime;
	}
	for(i = 0; i < argc; i++)
		ret |= _touch_do(prefs, argv[i], atime, mtime);
	return ret;
}

static int _touch_error(char const * message, int ret)
{
	fputs("touch: ", stderr);
	perror(message);
	return ret;
}

static int _touch_rtime(char const * filename, time_t * atime, time_t * mtime)
{
	struct stat st;

	if(stat(filename, &st) != 0)
		return _touch_error(filename, 1);
	*atime = st.st_atime;
	*mtime = st.st_mtime;
	return 0;
}

static int _touch_do(Prefs * prefs, char const * filename, time_t atime,
		time_t mtime)
{
	struct stat st;
	struct utimbuf ut;
	int fd;

#ifdef DEBUG
	fprintf(stderr, "%s%d%s%s%s%ld%s%ld%s", "_touch_do(", prefs->flags,
			", ", filename, ", ", atime, ", ", mtime, ");\n");
#endif
	if(!(prefs->flags & TOUCH_PREFS_c))
	{
		if((fd = open(filename, O_CREAT, 0666) == -1))
			_touch_error(filename, 0);
		else if(close(fd) != 0)
			_touch_error(filename, 0);
	}
	if(prefs->flags == TOUCH_PREFS_m || prefs->flags == TOUCH_PREFS_a)
		if(stat(filename, &st) != 0)
		{
			if(prefs->flags == TOUCH_PREFS_m)
				atime = st.st_atime;
			else
				mtime = st.st_mtime;
		}
	ut.actime = atime;
	ut.modtime = mtime;
	if(utime(filename, &ut) != 0)
	{
		if((prefs->flags & TOUCH_PREFS_c) && errno == ENOENT)
			return 0;
		return _touch_error(filename, 1);
	}
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: touch [-acm][-r ref_file|-t time] file...\n\
  -a	Change the access time\n\
  -c	Do not create file if it doesn't exist\n\
  -m	Change the modification time\n\
  -r	Use the time of the given file\n\
  -t	Use the specified time as [[CC]YY]MMDDhhmm[.SS]\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;

	if(_prefs_parse(&prefs, argc, argv) != 0 || argc - optind == 0)
		return _usage();
	return (_touch(&prefs, argc - optind, &argv[optind]) == 0) ? 0 : 2;
}
