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
specparam [@$*#?!0-]
word [^ \t;|&<>]+

%x DOLLAR EXPN

%%

[ \t\r\n]+	     ;
"&"		     { return AMPR; }
";"		     { return SEMI; }
";;"		     { return DSEMI; }
"||"		     { return DISJ; }
"&&" 		     { return CONJ; }

"="		     { return EQUAL; }
"|"		     { return PIPE; }

[0-9]+/">"	     { yylval.numval = atoi(yytext); return DIGIT_REDIR; }
[0-9]+/"<"	     { yylval.numval = atoi(yytext); return DIGIT_REDIR; }
[0-9]+/">>"	     { yylval.numval = atoi(yytext); return DIGIT_REDIR; }
[0-9]+/">|"	     { yylval.numval = atoi(yytext); return DIGIT_REDIR; }
[0-9]+/"<<"	     { yylval.numval = atoi(yytext); return DIGIT_REDIR; }
[0-9]+/"<<<"	     { yylval.numval = atoi(yytext); return DIGIT_REDIR; }
[0-9]+/">&"	     { yylval.numval = atoi(yytext); return DIGIT_REDIR; }
[0-9]+/"<&"	     { yylval.numval = atoi(yytext); return DIGIT_REDIR; }

"for"		     { return KW_FOR; }
"while"		     { return KW_WHILE; }
"do"		     { return KW_DO; }
"done"		     { return KW_DONE; }
"in"		     { return KW_IN; }
"case"		     { return KW_CASE; }
"esac"		     { return KW_ESAC; }
"if"		     { return KW_IF; }
"elif"		     { return KW_ELIF; }
"else"		     { return KW_ELSE; }
"then"		     { return KW_THEN; }
"fi"	             { return KW_FI;  }
"until"		     { return KW_UNTIL; }

">"		     { return LANGLE; }
"<"		     { return RANGLE; }
">>" 		     { return APPEND; }
">|"		     { return NCLBR;  }
"<<"		     { return HEREDOC; }
"<<<"		     { return HERESTR; }
">&"		     { return DUPOUT;  }
"<&"                 { return DUPIN;   }

"$"		     { BEGIN DOLLAR; }

<DOLLAR>[1-9]+	     { yylval.numval = atoi(yytext); BEGIN INITIAL; return ARGNUM; }
<DOLLAR>{specparam}  { yylval.paramval = yytext[0]; BEGIN INITIAL; return SPECPARAM; }
<DOLLAR>{identifier} { yylval.idval = gc_strndup(yytext, yyleng); BEGIN INITIAL; return PARAM_IDENTIFIER; }
<DOLLAR>"{"	     { BEGIN EXPN; return EXPN_START; }

<EXPN>"}"	     { BEGIN INITIAL; return EXPN_END; }
<EXPN>{identifier}   { return EXPN_IDENTIFIER; }
<EXPN>{word}	     { return EXPN_WORD;  }
<EXPN>":"	     { return EXPN_COLON; }
<EXPN>"-"	     { return EXPN_DASH; }
<EXPN>"="	     { return EXPN_EQUAL; }
<EXPN>"?"	     { return EXPN_QMARK; }
<EXPN>"+"	     { return EXPN_PLUS; }
<EXPN>"%"	     { return EXPN_PERCENT; }
<EXPN>"#"	     { return EXPN_POUND; }
<EXPN>":-"	     { return EXPN_COLON_DASH; }
<EXPN>":="	     { return EXPN_COLON_EQUAL; }
<EXPN>":?"	     { return EXPN_COLON_QMARK; }
<EXPN>":+"	     { return EXPN_COLON_PLUS; }
<EXPN>"%%"	     { return EXPN_DPERCENT; }
<EXPN>"##"	     { return EXPN_DPOUND; }

{ident}/"="	     { yylval.idval = gc_strndup(yytext, yyleng);
			return ANCHORED_IDENTIFIER; 	}
{ident}/"()"	     { yylval.idval = gc_strndup(yytext, yyleng);
                        return FNNAME_IDENTIFIER;       }
{word} 		     { yylval.wordval = new_ast_word((uint8_t*)yytext, yyleng);
			return WORD; 			}


%%

int yywrap(void) { return 1; }

