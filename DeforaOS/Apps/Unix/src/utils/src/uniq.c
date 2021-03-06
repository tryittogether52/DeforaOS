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



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* types */
typedef int Prefs;
#define UNIQ_PREFS_c 1
#define UNIQ_PREFS_d 2
#define UNIQ_PREFS_u 4


/* uniq */
/* PRE	if in == NULL then out == NULL too
 * POST
 * 	0	success
 * 	else	error(s) occured */
static int _uniq_error(char const * message, int ret);
static int _uniq_do(Prefs prefs, char const * fields, unsigned int skip,
		FILE * infp, FILE * outfp);

static int _uniq(Prefs prefs, char const * fields, unsigned int skip,
		char const * in, char const * out)
{
	FILE * infp = stdin;
	FILE * outfp = stdout;
	int ret;

	if(in != NULL && (infp = fopen(in, "r")) == NULL)
		return _uniq_error(in, 1);
	if(out != NULL && (outfp = fopen(out, "w")) == NULL)
	{
		fclose(infp);
		return _uniq_error(out, 1);
	}
	ret = _uniq_do(prefs, fields, skip, infp, outfp);
	if(in != NULL)
	{
		if(fclose(infp) != 0)
			ret = _uniq_error(in, 1);
		if(out != NULL && fclose(outfp) != 0)
			return _uniq_error(out, 1);
	}
	return ret;
}

static int _uniq_error(char const * message, int ret)
{
	fputs("uniq: ", stderr);
	perror(message);
	return ret;
}

static void _do_count(Prefs prefs, unsigned int skip, char * line, FILE * fp);
static int _uniq_do(Prefs prefs, char const * fields, unsigned int skip,
		FILE * infp, FILE * outfp)
{
	int ret = 0;
#define BUF 80
	char * line = NULL;
	int len = 0;
	char * p;

	for(;;)
	{
		if((p = realloc(line, len + BUF + 1)) == NULL)
		{
			free(line);
			ret = _uniq_error("malloc", 1);
			break;
		}
		line = p;
		if(fgets(&line[len], BUF + 1, infp) == NULL)
		{
			if(!feof(infp))
				ret = _uniq_error("fread", 1);
			break;
		}
		for(p = &line[len]; *p != '\0' && *p != '\n'; p++);
		len += BUF;
		if(p == line + BUF)
			continue;
		if(*p == '\n')
			*p = '\0';
#ifdef DEBUG
		fprintf(stderr, "%s%s%s", "DEBUG: Got line \"", line, "\"\n");
#endif
		_do_count(prefs, skip, line, outfp);
		line = NULL;
		len = 0;
	}
	_do_count(prefs, skip, NULL, outfp);
	return ret;
}

static int _count_repeated(char * lastline, char * line, unsigned int skip);
static void _do_count(Prefs prefs, unsigned int skip, char * line, FILE * fp)
{
	static char * lastline = NULL;
	static unsigned int cnt = 1;

	if(lastline == NULL)
	{
		lastline = line;
		cnt = 1;
		return;
	}
	if(line != NULL && _count_repeated(lastline, line, skip))
	{
		free(line);
		cnt++;
		return;
	}
	if(cnt == 1 && !(prefs & UNIQ_PREFS_d)) /* line is not repeated */
		fprintf(fp, "%s%s\n", prefs & UNIQ_PREFS_c ? "1 " : "",
				lastline);
	else if(cnt > 1 && !(prefs & UNIQ_PREFS_u)) /* line is repeated */
	{
		if(prefs & UNIQ_PREFS_c)
			fprintf(fp, "%d ", cnt);
		fprintf(fp, "%s\n", lastline);
	}
	free(lastline);
	lastline = line;
	cnt = 1;
}

/* PRE	line and lastline are valid strings
 * POST */
static int _count_repeated(char * lastline, char * line, unsigned int skip)
{
	if(strlen(lastline) < skip)
		return strlen(line) < skip;
	if(strlen(line) < skip)
		return 0;
	if(strcmp(&lastline[skip], &line[skip]) == 0)
		return 1;
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: uniq [-c|-d|-u][-f fields][-s char][input_file\
 [output_file]]\n\
  -c	Precede each output line with a count of the repetitions for the line\n\
  -d	Suppress the writing of lines that are not repeated\n\
  -s	Ignore the first char characters when doing comparisons\n\
  -u	Suppress the writing of lines that are repeated\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs = 0;
	char * fields = NULL;
	int skip = 0;
	char * p;
	char * in = NULL;
	char * out = NULL;
	int o;

	while((o = getopt(argc, argv, "cduf:s:")) != -1)
		switch(o)
		{
			case 'c':
				prefs |= UNIQ_PREFS_c;
				break;
			case 'd':
				prefs |= UNIQ_PREFS_d;
				break;
			case 's':
				skip = strtol(optarg, &p, 10);
				if(*optarg == '\0' || *p != '\0' || skip < 0)
					return _usage();
				break;
			case 'u':
				prefs |= UNIQ_PREFS_u;
				break;
			case 'f':
				/* FIXME */
			default:
				return _usage();
		}
	if(argc - optind >= 1)
	{
		in = argv[optind];
		if(argc - optind == 2)
			out = argv[optind+1];
		else if(argc - optind > 2)
			return _usage();
	}
	return (_uniq(prefs, fields, skip, in, out) == 0) ? 0 : 2;
}
