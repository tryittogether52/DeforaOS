/* tokencode.c */



#include <stdlib.h>
#include "tokencode.h"


/* TokenCode */
char * sTokenCode[TC_NULL] = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"\n",
	NULL,
	"&&",
	"||",
	";;",
	"<<",
	">>",
	"<&",
	">&",
	"<>",
	"<<-",
	">|",
	"&",
	"|",
	";",
	"<",
	">",
	"if",
	"then",
	"else",
	"elif",
	"fi",
	"do",
	"done",
	"case",
	"esac",
	"while",
	"until",
	"for",
	"{",
	"}",
	"!",
	"in"
};


/* TokenCode sets */
TokenCode CS_AND_OR[]		= {
	TC_RW_BANG,
	/* command */
	TC_TOKEN,
	TC_NULL
};
TokenCode CS_CMD_NAME[]		= {
	TC_WORD,
	TC_NULL
};
TokenCode CS_CMD_PREFIX[]	= {
	TC_IO_NUMBER,
	TC_NULL
};
TokenCode CS_CMD_SUFFIX[]	= {
	/* io_redirect */
	TC_IO_NUMBER,
	/* WORD */
	TC_WORD,
	TC_NULL
};
TokenCode CS_CMD_WORD[]		= {
	TC_WORD,
	TC_NULL
};
TokenCode CS_COMMAND[]		= {
	/* simple_command */
	TC_TOKEN,
	/* compound_command */
	TC_RW_LBRACE, /* SUBSHELL "(", */ TC_RW_FOR, TC_RW_CASE, TC_RW_IF, TC_RW_WHILE, TC_RW_UNTIL,
	/* function_definition */
	TC_NULL
};
TokenCode CS_COMPOUND_COMMAND[]	= {
	TC_RW_LBRACE, /* SUBSHELL "(", */
	TC_RW_FOR, TC_RW_CASE, TC_RW_IF, TC_RW_WHILE, TC_RW_UNTIL,
	TC_NULL
};
TokenCode CS_COMPOUND_LIST[]	= {
	TC_NULL
}; /* FIXME */
TokenCode CS_DO_GROUP[]		= {
	TC_NULL
}; /* FIXME */
TokenCode CS_FUNCTION_BODY[]	= {
	TC_NULL
}; /* FIXME */
TokenCode CS_FUNCTION_DEFINITION[] = {
	TC_NULL
}; /* FIXME */
TokenCode CS_IN[]		= {
	TC_RW_IN,
	TC_NULL
};
TokenCode CS_IO_FILE[]		= {
	TC_NULL
}; /* FIXME */
TokenCode CS_IO_HERE[]		= {
	TC_NULL
}; /* FIXME */
TokenCode CS_IO_REDIRECT[]	= {
	TC_IO_NUMBER,
	TC_NULL
}; /* FIXME */
TokenCode CS_LINEBREAK[]	= {
	TC_NEWLINE,
	TC_NULL
}; /* FIXME */
TokenCode CS_LIST[]		= {
	/* and_or */
	TC_RW_BANG, TC_TOKEN,
	/* separator_op */
	TC_OP_AMPERSAND, TC_OP_SEMICOLON,
	TC_NULL
};
TokenCode CS_NAME[]		= {
	TC_NAME,
	TC_NULL
};
TokenCode CS_NEWLINE_LIST[]	= {
	TC_NEWLINE,
	TC_NULL
};
TokenCode CS_PIPE_SEQUENCE[]	= {
	TC_NULL
}; /* FIXME */
TokenCode CS_PIPELINE[]		= {
	TC_RW_BANG,
	TC_NULL
}; /* FIXME */
TokenCode CS_REDIRECT_LIST[]	= {
	TC_NULL
}; /* FIXME */
TokenCode CS_SEPARATOR[]		= {
	TC_NEWLINE,
	TC_NULL
}; /* FIXME */
TokenCode CS_SEPARATOR_OP[]	= {
	TC_OP_AMPERSAND,
	TC_OP_SEMICOLON,
	TC_NULL
};
TokenCode CS_SEQUENTIAL_SEP[]	= {
	TC_NULL
}; /* FIXME */
TokenCode CS_SIMPLE_COMMAND[]	= {
	/* cmd_prefix */
	TC_IO_NUMBER,
	/* cmd_name */
	TC_WORD,
	TC_NULL
};
TokenCode CS_TERM[]		= {
	/* and_or */
	TC_RW_BANG,
	TC_TOKEN,
	TC_NULL
};
TokenCode CS_WORDLIST[]		= {
	TC_WORD,
	TC_NULL
};
