/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "as.h"
#include "token.h"


/* scan */
static Token * _scan_colon(FILE * fp, int * la);
static Token * _scan_comma(FILE * fp, int * la);
static Token * _scan_comment(FILE * fp, int * la);
static Token * _scan_dot(FILE * fp, int * la);
static Token * _scan_eof(int * la);
static Token * _scan_immediate(FILE * fp, int * la);
static Token * _scan_newline(FILE * fp, int * la);
static Token * _scan_number(FILE * fp, int * la);
static Token * _scan_register(FILE * fp, int * la);
static Token * _scan_space(FILE * fp, int * la);
static Token * _scan_word(FILE * fp, int * la);
Token * scan(FILE * fp)
{
	Token * t;
	static int la = EOF;

	if(la == EOF)
		la = fgetc(fp);
	if((t = _scan_colon(fp, &la))
			|| (t = _scan_comma(fp, &la))
			|| (t = _scan_comment(fp, &la))
			|| (t = _scan_dot(fp, &la))
			|| (t = _scan_eof(&la))
			|| (t = _scan_immediate(fp, &la))
			|| (t = _scan_newline(fp, &la))
			|| (t = _scan_number(fp, &la))
			|| (t = _scan_register(fp, &la))
			|| (t = _scan_space(fp, &la))
			|| (t = _scan_word(fp, &la)))
		return t;
	return NULL;
}

static Token * _scan_colon(FILE * fp, int * la)
{
	if(*la == ':')
	{
		*la = fgetc(fp);
		return token_new(TC_COLON, ":");
	}
	return NULL;
}

static Token * _scan_comma(FILE * fp, int * la)
{
	if(*la == ',')
	{
		*la = fgetc(fp);
		return token_new(TC_COMMA, ",");
	}
	return NULL;
}

static Token * _scan_comment(FILE * fp, int * la)
{
	int c;

	if(*la != '/')
		return NULL;
	if((c = fgetc(fp)) != '/')
	{
		fseek(fp, -1, SEEK_CUR);
		return NULL;
	}
	for(;;)
	{
		c = fgetc(fp);
		switch(c)
		{
			case '\r':
			case '\n':
			case EOF:
				*la = c;
				return token_new(TC_SPACE, "//");
		}
	}
}

static Token * _scan_dot(FILE * fp, int * la)
{
	if(*la == '.')
	{
		*la = fgetc(fp);
		return token_new(TC_DOT, ".");
	}
	return NULL;
}

static Token * _scan_eof(int * la)
{
	if(*la == EOF)
		return token_new(TC_EOF, "EOF");
	return NULL;
}

static Token * _scan_immediate(FILE * fp, int * la)
{
	char * str = NULL;
	int len = 1;
	char * p;
	Token * t;
	int hex = 0;

	if(*la != '$' || !isdigit(*la = fgetc(fp)) || (str = malloc(1)) == NULL)
		return NULL;
	str[0] = '$';
	do
	{
		if((p = realloc(str, len+2)) == NULL)
		{
			as_error("malloc", 0);
			free(str);
			return NULL;
		}
		str = p;
		str[len++] = *la;
		*la = fgetc(fp);
		if(len == 2 && str[1] == '0' && *la == 'x')
			hex = 1;
	}
	while(isdigit(*la) || (len == 2 && hex) || (hex && (tolower(*la) >= 'a'
					&& tolower(*la) <= 'f')));
	str[len] = '\0';
	t = token_new(TC_IMMEDIATE, str);
	free(str);
	return t;
}

static Token * _scan_newline(FILE * fp, int * la)
{
	if(*la == '\n' || *la == '\r') /* FIXME '\r\n' */
	{
		*la = fgetc(fp);
		return token_new(TC_NEWLINE, "\n");
	}
	return NULL;
}

static Token * _scan_number(FILE * fp, int * la)
{
	char * str = NULL;
	int len = 0;
	char * p;
	Token * t;

	if(!isdigit(*la))
		return NULL;
	do
	{
		if((p = realloc(str, len+2)) == NULL)
		{
			as_error("malloc", 0);
			free(str);
			return NULL;
		}
		str = p;
		str[len++] = *la;
		*la = fgetc(fp);
	}
	while(isdigit(*la));
	str[len] = '\0';
	t = token_new(TC_NUMBER, str);
	free(str);
	return t;
}

static Token * _scan_register(FILE * fp, int * la)
{
	char * str = NULL;
	int len = 1;
	char * p;
	Token * t;

	if(*la != '%' || !islower(*la = fgetc(fp)) || (str = malloc(1)) == NULL)
		return NULL;
	str[0] = '%';
	do
	{
		if((p = realloc(str, len+2)) == NULL)
		{
			free(str);
			return NULL; /* FIXME report error */
		}
		str = p;
		str[len++] = *la;
		*la = fgetc(fp);
	}
	while(islower(*la));
	str[len] = '\0';
	t = token_new(TC_REGISTER, str);
	free(str);
	return t;
}

static Token * _scan_space(FILE * fp, int * la)
{
	if(isspace(*la))
	{
		for(; isspace(*la); *la = fgetc(fp));
		return token_new(TC_SPACE, " ");
	}
	return NULL;
}

static Token * _scan_word(FILE * fp, int * la)
	/* FIXME */
{
	char * str = NULL;
	int len = 0;
	char * p;
	Token * t;

	if(!islower(*la))
		return NULL;
	do
	{
		if((p = realloc(str, len+2)) == NULL)
		{
			free(str);
			return NULL; /* FIXME report error */
		}
		str = p;
		str[len++] = *la;
		*la = fgetc(fp);
	}
	while(islower(*la) || isdigit(*la));
	str[len] = '\0';
	t = token_new(TC_WORD, str);
	free(str);
	return t;
}


Token * check(FILE * fp, TokenCode code)
{
	Token * t;

	if((t = scan(fp)) == NULL)
		return 0;
	if(t->code == code)
		return t;
	token_delete(t);
	return NULL;
}
