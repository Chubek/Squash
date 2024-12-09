%{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "common.h"
#include "memory.h"
#include "job.h"
#include "lexer.h"
#include "absyn.h"

extern bool do_exit;

void run_command_chain(Command *);
void yyerror(const char *);

void walk_tree(command_def);

static Command *cmd_chain = NULL;

%}

%define parse.trace

%union {
  const char *wordval;
  const char *idval;
  long numval;
  redirection_def redirval;
  command_def cmdval;
  term_def termval;
}

%token WORD ANCHORED_IDENTIFIER DOLLAR_IDENTIFIER INTEGER_NUMBER
%token SEMI AMPR DISJ CONJ PIPE

%type <redirval> redir
%type <cmdval> command simple_command pipeline
%type <termval> term

%type <wordval> WORD

%start squash

%%

squash: %empty
      | simple_command SEMI		{ walk_tree($1); }
      ;

simple_command: simple_command redir    { GET_simplecmd_redir($1) = $2; }
	      | simple_command WORD     { append_command_simplecmd_argv_field($1, (string_t)$2); }
	      | WORD			{ $$ = create_simplecmd(NULL, (string_t*)&$1, 1); }
	      ;

redir: '>' WORD 			{ $$ = create_intordr((string_t)$2); }
     | '<' WORD				{ $$ = create_outtordr((string_t)$2); }
     | ">>" WORD			{ $$ = create_appendrdr((string_t)$2); }
     ;

%%

void yyerror(const char *msg) {
  fprintf(stderr, "Parsing error occurred:\n");
  fprintf(stderr, "%s\n", msg);
}

void run_command_chain(Command *chain) {
  launch_job(chain, false);
}

void walk_tree(command_def tree) {
  if (tree->kind == SIMPLECMD_kind)
    printf("SimpleCMD\n");
}

