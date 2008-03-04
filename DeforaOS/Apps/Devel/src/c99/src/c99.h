/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel c99 */
/* c99 is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * c99 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with c99; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#ifndef C99_C99_H
# define C99_C99_H

# include <stdio.h>
# include <cpp.h>


/* C99 */
/* protected */
/* types */
typedef struct _C99 C99;

typedef enum _C99Code
{
	C99_CODE_NULL			= CPP_CODE_NULL,
	C99_CODE_COMMA			= CPP_CODE_COMMA,
	C99_CODE_DQUOTE			= CPP_CODE_DQUOTE,
	C99_CODE_META_DEFINE		= CPP_CODE_META_DEFINE,
	C99_CODE_META_ELIF		= CPP_CODE_META_ELIF,
	C99_CODE_META_ELSE		= CPP_CODE_META_ELSE,
	C99_CODE_META_ENDIF		= CPP_CODE_META_ENDIF,
	C99_CODE_META_ERROR		= CPP_CODE_META_ERROR,
	C99_CODE_META_IF		= CPP_CODE_META_IF,
	C99_CODE_META_IFDEF		= CPP_CODE_META_IFDEF,
	C99_CODE_META_IFNDEF		= CPP_CODE_META_IFNDEF,
	C99_CODE_META_INCLUDE		= CPP_CODE_META_INCLUDE,
	C99_CODE_META_LINE		= CPP_CODE_META_LINE,
	C99_CODE_META_PRAGMA		= CPP_CODE_META_PRAGMA,
	C99_CODE_META_UNDEF		= CPP_CODE_META_UNDEF,
	C99_CODE_META_WARNING		= CPP_CODE_META_WARNING,
	C99_CODE_OPERATOR_AEQUALS	= CPP_CODE_OPERATOR_AEQUALS,
	C99_CODE_OPERATOR_AMPERSAND	= CPP_CODE_OPERATOR_AMPERSAND,
	C99_CODE_OPERATOR_BAR		= CPP_CODE_OPERATOR_BAR,
	C99_CODE_OPERATOR_BEQUALS	= CPP_CODE_OPERATOR_BEQUALS,
	C99_CODE_OPERATOR_COLON		= CPP_CODE_OPERATOR_COLON,
	C99_CODE_OPERATOR_DAMPERSAND	= CPP_CODE_OPERATOR_DAMPERSAND,
	C99_CODE_OPERATOR_DBAR		= CPP_CODE_OPERATOR_DBAR,
	C99_CODE_OPERATOR_DEQUALS	= CPP_CODE_OPERATOR_DEQUALS,
	C99_CODE_OPERATOR_DGEQUALS	= CPP_CODE_OPERATOR_DGEQUALS,
	C99_CODE_OPERATOR_DGREATER	= CPP_CODE_OPERATOR_DGREATER,
	C99_CODE_OPERATOR_DHASH		= CPP_CODE_OPERATOR_DHASH,
	C99_CODE_OPERATOR_DIVIDE	= CPP_CODE_OPERATOR_DIVIDE,
	C99_CODE_OPERATOR_DLEQUALS	= CPP_CODE_OPERATOR_DLEQUALS,
	C99_CODE_OPERATOR_DLESS		= CPP_CODE_OPERATOR_DLESS,
	C99_CODE_OPERATOR_DMINUS	= CPP_CODE_OPERATOR_DMINUS,
	C99_CODE_OPERATOR_DOT		= CPP_CODE_OPERATOR_DOT,
	C99_CODE_OPERATOR_DOTDOTDOT	= CPP_CODE_OPERATOR_DOTDOTDOT,
	C99_CODE_OPERATOR_DPLUS		= CPP_CODE_OPERATOR_DPLUS,
	C99_CODE_OPERATOR_EQUALS	= CPP_CODE_OPERATOR_EQUALS,
	C99_CODE_OPERATOR_GEQUALS	= CPP_CODE_OPERATOR_GEQUALS,
	C99_CODE_OPERATOR_GREATER	= CPP_CODE_OPERATOR_GREATER,
	C99_CODE_OPERATOR_HASH		= CPP_CODE_OPERATOR_HASH,
	C99_CODE_OPERATOR_INVERSE	= CPP_CODE_OPERATOR_INVERSE,
	C99_CODE_OPERATOR_LBRACE	= CPP_CODE_OPERATOR_LBRACE,
	C99_CODE_OPERATOR_LBRACKET	= CPP_CODE_OPERATOR_LBRACKET,
	C99_CODE_OPERATOR_LEQUALS	= CPP_CODE_OPERATOR_LEQUALS,
	C99_CODE_OPERATOR_LESS		= CPP_CODE_OPERATOR_LESS,
	C99_CODE_OPERATOR_LPAREN	= CPP_CODE_OPERATOR_LPAREN,
	C99_CODE_OPERATOR_MASK		= CPP_CODE_OPERATOR_MASK,
	C99_CODE_OPERATOR_MEQUALS	= CPP_CODE_OPERATOR_MEQUALS,
	C99_CODE_OPERATOR_MGREATER	= CPP_CODE_OPERATOR_MGREATER,
	C99_CODE_OPERATOR_MINUS		= CPP_CODE_OPERATOR_MINUS,
	C99_CODE_OPERATOR_MODEQUALS	= CPP_CODE_OPERATOR_MODEQUALS,
	C99_CODE_OPERATOR_MODULO	= CPP_CODE_OPERATOR_MODULO,
	C99_CODE_OPERATOR_MORE		= CPP_CODE_OPERATOR_MORE,
	C99_CODE_OPERATOR_NEQUALS	= CPP_CODE_OPERATOR_NEQUALS,
	C99_CODE_OPERATOR_NOT		= CPP_CODE_OPERATOR_NOT,
	C99_CODE_OPERATOR_OR		= CPP_CODE_OPERATOR_OR,
	C99_CODE_OPERATOR_PEQUALS	= CPP_CODE_OPERATOR_PEQUALS,
	C99_CODE_OPERATOR_PLUS		= CPP_CODE_OPERATOR_PLUS,
	C99_CODE_OPERATOR_QUESTION	= CPP_CODE_OPERATOR_QUESTION,
	C99_CODE_OPERATOR_RBRACE	= CPP_CODE_OPERATOR_RBRACE,
	C99_CODE_OPERATOR_RBRACKET	= CPP_CODE_OPERATOR_RBRACKET,
	C99_CODE_OPERATOR_RPAREN	= CPP_CODE_OPERATOR_RPAREN,
	C99_CODE_OPERATOR_SEMICOLON	= CPP_CODE_OPERATOR_SEMICOLON,
	C99_CODE_OPERATOR_TEQUALS	= CPP_CODE_OPERATOR_TEQUALS,
	C99_CODE_OPERATOR_TILDE		= CPP_CODE_OPERATOR_TILDE,
	C99_CODE_OPERATOR_TIMES		= CPP_CODE_OPERATOR_TIMES,
	C99_CODE_OPERATOR_XEQUALS	= CPP_CODE_OPERATOR_XEQUALS,
	C99_CODE_OPERATOR_XOR		= CPP_CODE_OPERATOR_XOR,
	C99_CODE_SQUOTE			= CPP_CODE_SQUOTE,
	C99_CODE_WHITESPACE		= CPP_CODE_WHITESPACE,
	C99_CODE_WORD			= CPP_CODE_WORD,
	C99_CODE_KEYWORD_AUTO,
	C99_CODE_KEYWORD_BREAK,
	C99_CODE_KEYWORD_CASE,
	C99_CODE_KEYWORD_CHAR,
	C99_CODE_KEYWORD_CONST,
	C99_CODE_KEYWORD_CONTINUE,
	C99_CODE_KEYWORD_DEFAULT,
	C99_CODE_KEYWORD_DO,
	C99_CODE_KEYWORD_DOUBLE,
	C99_CODE_KEYWORD_ELSE,
	C99_CODE_KEYWORD_ENUM,
	C99_CODE_KEYWORD_EXTERN,
	C99_CODE_KEYWORD_FLOAT,
	C99_CODE_KEYWORD_FOR,
	C99_CODE_KEYWORD_GOTO,
	C99_CODE_KEYWORD_IF,
	C99_CODE_KEYWORD_INLINE,
	C99_CODE_KEYWORD_INT,
	C99_CODE_KEYWORD_LONG,
	C99_CODE_KEYWORD_REGISTER,
	C99_CODE_KEYWORD_RESTRICT,
	C99_CODE_KEYWORD_RETURN,
	C99_CODE_KEYWORD_SHORT,
	C99_CODE_KEYWORD_SIGNED,
	C99_CODE_KEYWORD_SIZEOF,
	C99_CODE_KEYWORD_STATIC,
	C99_CODE_KEYWORD_STRUCT,
	C99_CODE_KEYWORD_SWITCH,
	C99_CODE_KEYWORD_TYPEDEF,
	C99_CODE_KEYWORD_UNION,
	C99_CODE_KEYWORD_UNSIGNED,
	C99_CODE_KEYWORD_VOID,
	C99_CODE_KEYWORD_VOLATILE,
	C99_CODE_KEYWORD_WHILE,
	C99_CODE_KEYWORD__BOOL,
	C99_CODE_KEYWORD__COMPLEX,
	C99_CODE_KEYWORD__IMAGINARY
} C99Code;

typedef struct _C99Prefs
{
	int flags;
	char const * outfile;
	const char ** paths;
	size_t paths_cnt;
	char ** defines;
	size_t defines_cnt;
	const char ** undefines;
	size_t undefines_cnt;
	int optlevel;
} C99Prefs;
# define C99PREFS_c 0x1
# define C99PREFS_E 0x2
# define C99PREFS_g 0x4
# define C99PREFS_s 0x8


/* functions */
C99 * c99_new(C99Prefs * prefs, char const * pathname);
int c99_delete(C99 * c99);

/* useful */
int c99_parse(C99 * c99);

int c99_scan(C99 * c99, Token ** token);

#endif /* !C99_C99_H */
