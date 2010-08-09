/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "common.h"
#include "parser.h"


/* private */
/* types */
typedef struct _State
{
	Cpp * cpp;
	Token * token;
	unsigned int error_cnt;
	unsigned int warning_cnt;
	Code * code;
	char * operator;
	AsOperand ** operands;
	size_t operands_cnt;
} State;


/* prototypes */
static int _parser_scan(State * state);
static int _parser_check(State * state, TokenCode code);
static int _parser_is_code(State * state, TokenCode code);
static int _parser_in_set(State * state, TokenSet set);

static int _parser_error(State * state, char const * format, ...);
static int _parser_warning(State * state, char const * format, ...);

/* grammar */
static int _program(State * state);
static int _newline(State * state);
static int _space(State * state);
static int _section_list(State * state);
static int _section(State * state);
static int _instruction_list(State * state);
static int _function(State * state);
static int _instruction(State * state);
static int _operator(State * state);
static int _operand_list(State * state);
static int _operand(State * state);


/* functions */
/* parser_scan */
static int _scan_skip_meta(State * state);

static int _parser_scan(State * state)
{
	int ret;
	TokenCode code;
	char const * string;

	if(state->token != NULL)
		token_delete(state->token);
	if((ret = _scan_skip_meta(state)) != 0
			|| state->token == NULL)
		return ret;
	code = token_get_code(state->token);
	string = token_get_string(state->token);
	if(code == AS_CODE_WORD)
	{
		if(string != NULL && string[0] == '$')
			token_set_code(state->token, AS_CODE_IMMEDIATE);
	}
	else if(code == AS_CODE_OPERATOR_MODULO)
	{
		/* FIXME ugly workaround */
		if((ret = _scan_skip_meta(state)) != 0)
			return ret;
		if(_parser_is_code(state, AS_CODE_WORD))
			token_set_code(state->token, AS_CODE_REGISTER);
	}
	return 0;
}

static int _scan_skip_meta(State * state)
{
	int ret = 0;
	TokenCode code;

	while(cpp_scan(state->cpp, &state->token) == 0)
	{
		if(state->token == NULL)
			return ret;
		if((code = token_get_code(state->token)) < AS_CODE_META_FIRST
				|| code > AS_CODE_META_LAST)
			return ret;
		if(code == AS_CODE_META_ERROR)
			ret |= _parser_error(state, "%s", token_get_string(
						state->token));
		else if(code == AS_CODE_META_WARNING)
			_parser_warning(state, "%s", token_get_string(
						state->token));
		token_delete(state->token);
	}
	return 1;
}


/* parser_check */
static int _parser_check(State * state, TokenCode code)
{
	int ret = 0;

	if(!_parser_is_code(state, code))
		ret = _parser_error(state, "%s%u", "Parse error: expected ",
				code);
	ret |= _parser_scan(state);
	return ret;
}


/* parser_is_code */
static int _parser_is_code(State * state, TokenCode code)
{
	if(state->token == NULL)
		return 0;
	return token_get_code(state->token) == code;
}


/* parser_in_set */
static int _parser_in_set(State * state, TokenSet set)
{
	if(state->token == NULL)
		return 0;
	return token_in_set(state->token, set);
}


/* parser_error */
static int _parser_error(State * state, char const * format, ...)
{
	va_list ap;

	fputs("as: ", stderr);
	if(state->cpp != NULL && state->token != NULL)
		fprintf(stderr, "%s%s%u: ", cpp_get_filename(state->cpp),
				", line ", token_get_line(state->token));
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	fputc('\n', stderr);
	return ++state->error_cnt;
}


/* parser_warning */
static int _parser_warning(State * state, char const * format, ...)
{
	va_list ap;

	fputs("as: ", stderr);
	if(state->cpp != NULL && state->token != NULL)
		fprintf(stderr, "%s%s%u: ", cpp_get_filename(state->cpp),
				", line ", token_get_line(state->token));
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	fputc('\n', stderr);
	return ++state->warning_cnt;
}


/* protected */
/* functions */
/* parser */
int parser(Code * code, char const * infile)
{
	CppPrefs prefs;
	State state;

	memset(&prefs, 0, sizeof(prefs));
	prefs.filename = infile;
	prefs.filters = CPP_FILTER_COMMENT;
	memset(&state, 0, sizeof(state));
	state.code = code;
	if((state.cpp = cpp_new(&prefs)) == NULL)
		return _parser_error(&state, "%s", error_get());
	if(_parser_scan(&state) != 0)
		return _parser_error(&state, "%s", error_get());
	if(_program(&state) != 0)
		error_set_code(1, "%s%s%u%s%u%s", infile,
				": Compilation failed with ", state.error_cnt,
				" error(s) and ", state.warning_cnt,
				" warning(s)");
	if(state.token != NULL)
		token_delete(state.token);
	free(state.operands);
	return state.error_cnt;
}


/* grammar */
/* program */
static int _program(State * state)
	/* { newline } section_list { newline } */
{
	int ret = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	while(_parser_in_set(state, TS_NEWLINE))
		ret |= _newline(state);
	ret |= _section_list(state);
	while(_parser_in_set(state, TS_NEWLINE))
		ret |= _newline(state);
	return ret;
}


/* newline */
static int _newline(State * state)
	/* [ space ] NEWLINE */
{
	int ret = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(_parser_in_set(state, TS_SPACE))
		ret |= _space(state);
	if(_parser_check(state, AS_CODE_NEWLINE) != 0)
		ret |= 1;
	return ret;
}


/* space */
static int _space(State * state)
	/* SPACE { SPACE } */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _parser_check(state, AS_CODE_WHITESPACE);
	while(_parser_is_code(state, AS_CODE_WHITESPACE))
		ret |= _parser_scan(state);
	return ret;
}


/* section_list */
static int _section_list(State * state)
	/* { section instruction_list } */
{
	int ret = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	while(_parser_in_set(state, TS_SECTION))
	{
		ret |= _section(state);
		ret |= _instruction_list(state);
	}
	return ret;
}


/* section */
static int _section(State * state)
	/* "." WORD newline */
{
	int ret;
	char const * string;
	size_t len;
	char * section = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _parser_check(state, AS_CODE_OPERATOR_DOT);
	string = (state->token != NULL) ? token_get_string(state->token) : NULL;
	if(string != NULL && _parser_is_code(state, AS_CODE_WORD))
	{
		len = strlen(token_get_string(state->token)) + 2;
		if((section = malloc(len)) == NULL)
			return ret | error_set_code(1, "%s", strerror(errno));
		snprintf(section, len, ".%s", token_get_string(state->token));
		ret |= _parser_scan(state);
	}
	else
		/* XXX what if code is AS_CODE_WORD but string is NULL? */
		ret |= _parser_check(state, AS_CODE_WORD);
	ret |= _newline(state);
	if(section != NULL)
	{
#ifdef DEBUG
		fprintf(stderr, "%s\"%s\"\n", "DEBUG: section ", section);
#endif
		if(code_section(state->code, section) != 0)
			ret |= _parser_error(state, "%s", error_get());
		free(section);
	}
	return ret;
}


/* instruction_list */
static int _instruction_list(State * state)
	/* { (function | space instruction | [space] newline) } */
{
	int ret = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	while(_parser_in_set(state, TS_INSTRUCTION_LIST))
		if(_parser_in_set(state, TS_FUNCTION))
			ret |= _function(state);
		else if(_parser_in_set(state, TS_SPACE))
		{
			ret |= _space(state);
			if(_parser_in_set(state, TS_INSTRUCTION))
				ret |= _instruction(state);
		}
		else if(_parser_in_set(state, TS_NEWLINE))
			ret |= _newline(state);
		else
			ret |= _parser_error(state, "%s", "Expected function"
					", instruction or linefeed");
	return ret;
}


/* function */
static int _function(State * state)
	/* WORD ":" newline */
{
	int ret;
	char const * string;
	char * function = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	string = token_get_string(state->token);
	if(_parser_is_code(state, AS_CODE_WORD) && string != NULL)
		if((function = strdup(string)) == NULL)
			return error_set_code(1, "%s", strerror(errno));
	ret = _parser_check(state, AS_CODE_WORD);
	ret |= _parser_check(state, AS_CODE_OPERATOR_COLON);
	ret |= _newline(state);
	if(function != NULL)
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s \"%s\"\n", "function", function);
#endif
		if(code_function(state->code, function) != 0)
			ret |= _parser_error(state, "%s", error_get());
		free(function);
	}
	return ret;
}


/* instruction */
static int _instruction(State * state)
	/* operator [ space [ operand_list ] ] newline */
{
	int ret;
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _operator(state);
	if(_parser_in_set(state, TS_SPACE))
	{
		ret |= _space(state);
		if(_parser_in_set(state, TS_OPERAND_LIST))
			ret |= _operand_list(state);
	}
	if(state->operator != NULL)
	{
		if(code_instruction(state->code, state->operator,
					state->operands, state->operands_cnt))
			ret |= _parser_error(state, "%s", error_get());
		free(state->operator);
		state->operator = NULL;
	}
	for(i = 0; i < state->operands_cnt; i++)
	{
		free(state->operands[i]->value);
		free(state->operands[i]);
	}
	/* optimized free(state->operands); out */
	state->operands_cnt = 0;
	return ret | _newline(state);
}


/* operator */
static int _operator(State * state)
	/* WORD */
{
	char const * string;
	char * operator = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((string = token_get_string(state->token)) == NULL)
	{
		_parser_scan(state);
		return 1;
	}
	if((operator = strdup(string)) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	/* optimized free(state->operator); out */
	state->operator = operator;
#ifdef DEBUG
	fprintf(stderr, "%s \"%s\"\n", "DEBUG: operator", operator);
#endif
	return _parser_scan(state);
}


/* operand_list */
static int _operand_list(State * state)
	/* operand [ space ] { "," [ space ] operand [ space ] } */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _operand(state);
	if(_parser_in_set(state, TS_SPACE))
		ret |= _space(state);
	while(_parser_is_code(state, AS_CODE_COMMA))
	{
		ret |= _parser_scan(state);
		if(_parser_in_set(state, TS_SPACE))
			ret |= _space(state);
		ret |= _operand(state);
		if(_parser_in_set(state, TS_SPACE))
			ret |= _space(state);
	}
	return ret;
}


/* operand */
static int _operand(State * state)
	/* WORD | NUMBER | IMMEDIATE | REGISTER
	 * | ( "[" [space] WORD [space] "]" ) */
{
	int ret = 0;
	TokenCode code;
	char const * string;
	AsOperand ** p;

	if(state->token == NULL)
		return 1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((code = token_get_code(state->token)) == AS_CODE_OPERATOR_LBRACKET)
	{
		ret = _parser_scan(state);
		if(_parser_in_set(state, TS_SPACE))
			ret |= _space(state);
	}
	if((string = token_get_string(state->token)) != NULL)
	{
#ifdef DEBUG
		fprintf(stderr, "%s%s\"\n", "DEBUG: new operand: \"",
				token_get_string(state->token));
#endif
		if((p = realloc(state->operands, (state->operands_cnt + 1)
						* sizeof(*p))) == NULL
				|| (p[state->operands_cnt] = malloc(
						sizeof(**p))) == NULL)
			ret |= _parser_error(state, "%s", strerror(errno));
		else
		{
			state->operands = p;
			p = &state->operands[state->operands_cnt];
			(*p)->type = token_get_code(state->token);
			(*p)->dereference = (code == AS_CODE_OPERATOR_LBRACKET);
			/* FIXME necessary already here? */
			(*p)->value = strdup(string);
			state->operands_cnt++;
		}
	}
	ret |= _parser_scan(state);
	if(code == AS_CODE_OPERATOR_LBRACKET)
	{

		if(_parser_in_set(state, TS_SPACE))
			ret |= _space(state);
		ret |= _parser_check(state, AS_CODE_OPERATOR_RBRACKET);
	}
	return ret;
}
