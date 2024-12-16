%{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "absyn.h"
#include "parser.tab.h"
#include "memory.h"

static ASTBuffer *current_string = NULL;
void append_char_to_current_string(char ch);
void blank_current_string(void);
void init_current_string(void);
%}

%option stack noyywrap

ident [a-zA-Z_][a-zA-Z0-9_]*
ndigit [1-9]
zdigit [0-9]
opt_ws [ \t\n\r]*
specparam [@$*#?!0-]
buffer [^ \t;|&<>(){}]+
expnpunct [:=?+%#-]{1,2}

%s DOLLAR EXPN SQOUTE DQOUTE TICK ARITH BRACK

%%

^"#".+$		     ;

[ \t]+		     ;
[\r\n]+		     { return NEWLINE; }

"("		     { return LPAREN; }
")"		     { return RPAREN; }
"{"		     { return LCURLY; }
"}"		     { return RCURLY; }

"()"		     { return FN_PARENS; }

"~"		     { return TILDE; }
"*"		     { return STAR; }
"?"		     { return QMARK; }
"!"		     { return BANG; }

"["		     { BEGIN BRACK; return BRACK_START; }

<BRACK>"-"	     { return BRACK_DASH; }
<BRACK>"!"	     { return BRACK_BANG; }
<BRACK>"\["	     { yylval.charval = '['; return BRACK_CHAR; }
<BRACK>"\*"	     { yylval.charval = '*'; return BRACK_CHAR; }
<BRACK>"\!"	     { yylval.charval = '!'; return BRACK_CHAR; }
<BRACK>"\?"	     { yylval.charval = '?'; return BRACK_CHAR; }
<BRACK>"\]"	     { yylval.charval = ']'; return BRACK_CHAR; }

<BRACK>[^!\]\[\-\?*] { yylval.charval = yytext[0]; return BRACK_CHAR; }

<BRACK>"]"	     { BEGIN INITIAL; return BRACK_END; }

"&"		     { return AMPR; }
";"		     { return SEMI; }
";;"		     { return DSEMI; }
"||"		     { return DISJ; }
"&&" 		     { return CONJ; }

"="		     { return EQUAL; }
"|"		     { return PIPE; }

"'"		     { BEGIN SQUOTE; }
"\""		     { BEGIN DQUOTE; return STRING_START; }

"$(("		     { yy_push_state(YYSTATE); BEGIN ARITH; return ARITH_START; }

<ARITH>"+"	     { return PLUS; }
<ARITH>"-"	     { return MINUS; }
<ARITH>"*"	     { return TIMES; }
<ARITH>"/"	     { return DIV; }
<ARITH>"%"	     { return MODULO; }
<ARITH>">>"	     { return SHR; }
<ARITH>"<<"	     { return SHL; }
<ARITH>"))"	     { yy_pop_state(); return ARITH_END; }
<ARITH>[1-9][0-9]*   { yylval.numval = atoi(yytext); return INTEGER; }


<INITIAL,DQUOTE>"`"  { yy_push_state(YYSTATE); BEGIN TICK; return TICK_START; }

<TICK,DQUOTE>"\$"	     { append_char_to_current_string('$'); }
<TICK,DQUOTE>"\`"	     { append_char_to_current_string('`'); }
<TICK,DQUOTE>"\\\""	     { append_char_to_current_string('"'); }
<TICK,DQUOTE>"\\"	     { append_char_to_current_string('\\'); }
<TICK,DQUOTE>"\\n"	     { append_char_to_current_string('\n'); }

<SQOUTE>[^']		     { append_char_to_current_string(yytext[0]); }
<DQUOTE>[^$`\\"]	     { append_char_to_current_string(yytext[0]); }

<SQUOTE>"'"		     { yy_pop_state(); 
			       yylval.bufferval = gc_incref(current_string); 
			       blank_current_string(); 
			       return QSTRING; 
			     }
<DQUOTE>"\""		     { yy_pop_state(); 
			       yylval.bufferval = gc_incref(current_string);
			       blank_current_string();
			       return STRING_END; }
<TICK>"`"		     { yy_pop_state(); return TICK_END; }

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

<INITIAL,DQUOTE,TICK>"$" { yy_push_state(YYSTATE); BEGIN DOLLAR; }

<DOLLAR>[1-9]+	     { yylval.numval = atoi(yytext); yy_pop_state(); return ARGNUM; }
<DOLLAR>{specparam}  { yylval.paramval = yytext[0]; yy_pop_state(); return SPECPARAM; }
<DOLLAR>{ident}      { yylval.bufferval = new_ast_buffer(yytext, yyleng); yy_pop_state(); return PARAM_IDENTIFIER; }
<DOLLAR>"{"	     { yy_push_state(YYSTATE); BEGIN EXPN; return EXPN_START; }


<EXPN>"}"	     { yy_pop_state(); return EXPN_END; }
<EXPN>{ident} 	     { yylval.bufferval = new_ast_buffer((uint8_t*)yytext, yyleng); 
			return EXPN_IDENTIFIER; }
<EXPN>{buffer}	     { yylval.bufferval = new_ast_buffer((uint8_t*)yytext, yyleng); 
			return EXPN_WORD;  }
<EXPN>{expnpunct}    { yylval.bufferval = new_ast_buffer((uint8_t*)yytext, yyleng);
			return EXPN_PUNCT; }

{ident}/"="	     { yylval.bufferval = new_ast_buffer((uint8_t*)yytext, yyleng);
			return ANCHORED_IDENTIFIER; 	}
{ident}/"()"         { yylval.bufferval = new_ast_buffer((uint8_t*)yytext, yyleng);
                        return FNNAME_IDENTIFIER;       }
{buffer} 		     { yylval.bufferval = new_ast_buffer((uint8_t*)yytext, yyleng);
			return WORD; 			}


%%

void append_char_to_current_string(char ch) {
  ast_buffer_append_char(current_string, ch);
}

void blank_current_string(void) {
  delete_ast_buffer(current_string);
  init_current_string();
}

void init_current_string(void) {
  current_string = new_ast_buffer_blank();
}

int yywrap(void) { return 1; }

