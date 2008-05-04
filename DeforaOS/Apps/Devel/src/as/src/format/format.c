/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel as */
/* as is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * as is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with as; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <System.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "as.h"
#include "format.h"
#include "../../config.h"


/* Format */
/* private */
/* types */
struct _Format
{
	char * arch;
	char * filename;
	FILE * fp;
	FormatPlugin * plugin;
	Plugin * handle;
};


/* private */
/* prototypes */
static int _format_init(Format * format);
static int _format_exit(Format * format);


/* functions */
/* format_init */
static int _format_init(Format * format)
{
	if(format->plugin->init == NULL)
		return 0;
	return format->plugin->init(format->fp, format->arch);
}


/* format_exit */
static int _format_exit(Format * format)
{
	if(format->plugin->exit == NULL)
		return 0;
	return format->plugin->exit(format->fp);
}


/* public */
/* functions */
/* format_new */
Format * format_new(char const * format, char const * arch,
		char const * filename, FILE * fp)
{
	Format * f;
	Plugin * handle;
	FormatPlugin * plugin;

	if(format == NULL)
		format = "elf"; /* XXX ask the arch plugin? */
	if((handle = plugin_new(LIBDIR, PACKAGE, "format", format)) == NULL)
		return NULL;
	if((plugin = plugin_lookup(handle, "format_plugin")) == NULL
			|| (f = object_new(sizeof(*f))) == NULL)
	{
		plugin_delete(handle);
		return NULL;
	}
	f->arch = string_new(arch);
	f->fp = fp;
	f->plugin = plugin;
	f->filename = string_new(filename);
	plugin->filename = f->filename;
	f->handle = handle;
	if(f->arch == NULL || f->filename == NULL || _format_init(f) != 0)
	{
		format_delete(f);
		return NULL;
	}
	return f;
}


/* format_delete */
int format_delete(Format * format)
{
	int ret;

	ret = _format_exit(format);
	plugin_delete(format->handle);
	free(format->filename);
	free(format->arch);
	object_delete(format);
	return ret;
}


/* useful */
/* format_function */
int format_function(Format * format, char const * function)
{
	if(format->plugin->function == NULL)
		return 0;
	return format->plugin->function(format->fp, function);
}


/* format_section */
int format_section(Format * format, char const * section)
{
	if(format->plugin->section == NULL)
		return 0;
	return format->plugin->section(format->fp, section);
}
