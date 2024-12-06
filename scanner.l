%{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "parser.tab.h"
#include "memory.h"
%}

ident [a-zA-Z_][a-zA-Z0-9_]*
ndigit [1-9]
zdigit [0-9]
opt_ws [ \t\n\r]*

%%

"exit"		     { return BUILTIN_EXIT; }

[ \t\r\n]+	     ;
";"		     { return SEMI; }
{ident}/"="	     { yylval.idval = gc_strndup(yytext, yyleng);
			return ANCHORED_IDENTIFIER; 	}
"$"{ident}           { yylval.idval = gc_strndup(yytext, yyleng);
			return DOLLAR_IDENTIFIER; 	}
[0-9]+		     { yylval.numval = strtol(gc_strndup(yytext, yyleng), NULL, 10);
			return INTEGER_NUMBER;     	}
[^ \t;|&]+	     { yylval.wordval = gc_strndup(yytext, yyleng);
			return WORD; 			}


%%

int yywrap(void) { return 1; }

