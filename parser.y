%{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "types.h"
#include "memory.h"
#include "job.h"
#include "lexer.h"

void run_command_chain(Command *);
void yyerror(const char *);

static Command *cmd_chain = NULL;

%}

%union {
  const char *wordval;
  const char *idval;
  long numval;
}

%token WORD ANCHORED_IDENTIFIER DOLLAR_IDENTIFIER INTEGER_NUMBER

%start input

%%

input: input repl
     | repl
     ;

repl: command '\n'	{ run_command_chain(cmd_chain); }
    ;

command: WORD command   { add_argv(cmd_chain, yylval.wordval); }
       | WORD		{ cmd_chain = new_command(); add_argv(cmd_chain, yylval.wordval); }
       ;

%%

void yyerror(const char *msg) {
  fprintf(stderr, "Parsing error occurred:\n");
  fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
}

void run_command_chain(Command *chain) {
  launch_job(chain, false);
}

