%{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "types.h"
#include "memory.h"
#include "job.h"
#include "lexer.h"

extern bool do_exit;

void run_command_chain(Command *);
void yyerror(const char *);

static Command *cmd_chain = NULL;

%}

%define parse.trace

%union {
  const char *wordval;
  const char *idval;
  long numval;
}

%token WORD ANCHORED_IDENTIFIER DOLLAR_IDENTIFIER INTEGER_NUMBER SEMI

%token BUILTIN_EXIT

%start repl

%%

repl: %empty
    | command SEMI	{ run_command_chain(cmd_chain); }
    | BUILTIN_EXIT SEMI	{ do_exit = true; }
    ;

command: command WORD   { add_argv(cmd_chain, yylval.wordval); }
       | WORD		{ cmd_chain = new_command(); add_argv(cmd_chain, yylval.wordval); }
       ;

%%

void yyerror(const char *msg) {
  fprintf(stderr, "Parsing error occurred:\n");
  fprintf(stderr, "%s\n", msg);
}

void run_command_chain(Command *chain) {
  launch_job(chain, false);
}

