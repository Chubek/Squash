%{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "absyn.h"
#include "parser.tab.h"
#include "memory.h"
%}

ident [a-zA-Z_][a-zA-Z0-9_]*
ndigit [1-9]
zdigit [0-9]
opt_ws [ \t\n\r]*

%%

[ \t\r\n]+	     ;
[0-9]		     { yylval.numval = strtol(yytext, NULL, 10); return DIGIT; }
"&"		     { return AMPR; }
";"		     { return SEMI; }
"|"		     { return PIPE; }
"||"		     { return DISJ; }
"&&" 		     { return CONJ; }
">"		     { return LANGLE; }
"<"		     { return RANGLE; }
">>" 		     { return APPEND; }
">|"		     { return NCLBR;  }
"<<"		     { return HEREDOC; }
"<<<"		     { return HERESTR; }
">&"		     { return DUPOUT;  }
"<&"                 { return DUPIN;   }
{ident}/"="	     { yylval.idval = gc_strndup(yytext, yyleng);
			return ANCHORED_IDENTIFIER; 	}
"$"{ident}           { yylval.idval = gc_strndup(yytext, yyleng);
			return DOLLAR_IDENTIFIER; 	}
[^ \t;|&<>]+	     { yylval.wordval = new_ast_word((uint8_t*)yytext, yyleng);
			return WORD; 			}


%%

int yywrap(void) { return 1; }

