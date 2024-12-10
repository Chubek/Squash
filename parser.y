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

void yyerror(const char *);

void walk_tree(ASTSimpleCommand*);

%}

%define parse.trace

%union {
  const char *idval;
  long numval;
  ASTWord *wordval;
  ASTRedir *redirval;
  ASTCommand *cmdval;
  ASTSimpleCommand *simplecmdval;
}

%token WORD ANCHORED_IDENTIFIER DOLLAR_IDENTIFIER INTEGER_NUMBER
%token SEMI AMPR DISJ CONJ PIPE
%token LANGLE RANGLE APPEND

%type <cmdval> command pipeline
%type <simplecmdval> simple_command
%type <redirval> redir
%type <wordval> word_chain

%type <wordval> WORD

%start squash

%%

squash: %empty
      | SEMI				{ }
      | simple_command SEMI		{ walk_tree($1); }
      ;

simple_command: word_chain redir        { $$ = new_ast_simple_command($1); $$->redir = gc_incref($2); }
	      | word_chain		{ $$ = new_ast_simple_command($1);  }
	      ;

redir: APPEND WORD                      { $$ = new_ast_redir(REDIR_Append, $2); }
     | LANGLE WORD			{ $$ = new_ast_redir(REDIR_Out, $2);    }
     | RANGLE WORD                      { $$ = new_ast_redir(REDIR_In, $2);     }
     ;

word_chain: word_chain WORD 		{ ast_word_append($1, $2); }
	  | WORD			{ $$ = $1; }
	  ;

%%

void yyerror(const char *msg) {
  fprintf(stderr, "Parsing error occurred:\n");
  fprintf(stderr, "%s\n", msg);
}

void walk_tree(ASTSimpleCommand *cmd) {
 printf("%s\n", cmd->argv->buffer);
 printf("%s\n", cmd->redir->subj->buffer);
}

