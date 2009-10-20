/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix utils */
/* utils is not free software; you can redistribute it and/or modify it under
 * the terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * utils is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with utils; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <unistd.h>
#include <stdio.h>


/* logname */
static int _logname_error(char const * message, int ret);

static int _logname(void)
{
	char const * lgnm;

	if((lgnm = getlogin()) == NULL)
		return _logname_error("getlogin", 1);
	puts(lgnm);
	return 0;
}

static int _logname_error(char const * message, int ret)
{
	fputs("logname: ", stderr);
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: logname\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;

	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind != argc)
		return _usage();
	return (_logname() == 0) ? 0 : 2;
}
