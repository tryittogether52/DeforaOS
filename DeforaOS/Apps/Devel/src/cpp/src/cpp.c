/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel cpp */
/* cpp is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * cpp is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with cpp; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "parser.h"
#include "cpp.h"


/* Cpp */
/* private */
/* types */
struct _Cpp
{
	Parser ** parser;
	size_t parser_cnt;
	char error[PATH_MAX];
};


/* public */
/* cpp_new */
Cpp * cpp_new(void)
{
	Cpp * cpp;

	if((cpp = malloc(sizeof(*cpp))) == NULL)
		return NULL;
	memset(cpp, sizeof(*cpp), 0);
	return cpp;
}


/* cpp_delete */
void cpp_delete(Cpp * cpp)
{
	size_t i;

	for(i = 0; i < cpp->parser_cnt; i++)
		parser_delete(cpp->parser[i]);
	free(cpp->parser);
	free(cpp);
}


/* accessors */
/* cpp_get_error */
char const * cpp_get_error(Cpp * cpp)
{
	return cpp->error;
}


/* useful */
int cpp_parse(Cpp * cpp, char const * pathname)
{
	Parser ** p;

	if((p = realloc(cpp->parser, (cpp->parser_cnt + 1)
					* sizeof(*p))) == NULL)
		return 1; /* FIXME get error */
	cpp->parser = p;
	if((cpp->parser[cpp->parser_cnt] = parser_new(pathname)) == NULL)
		return 1; /* FIXME get error */
	cpp->parser_cnt++;
	return 0;
}


/* cpp_read */
ssize_t cpp_read(Cpp * cpp, char * buf, size_t cnt)
{
	/* FIXME implement */
	return -1;
}
