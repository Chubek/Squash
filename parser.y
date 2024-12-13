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

void walk_tree(ASTPipeline*);

%}

%define parse.trace

%union {
  const char *idval;
  int numval;
  ASTWord *wordval;
  ASTRedir *redirval;
  ASTCommand *cmdval;
  ASTSimpleCommand *simplecmdval;
  ASTPipeline *pipelineval;
  ASTShtoken *shtokenval;
  enum CompoundKind listkindval;
}

%token WORD ANCHORED_IDENTIFIER DOLLAR_IDENTIFIER
%token SEMI AMPR DISJ CONJ PIPE EQUAL
%token LANGLE RANGLE APPEND DUPIN DUPOUT NCLBR HERESTR HEREDOC
%token DIGIT_REDIR

%type <cmdval> command
%type <simplecmdval> simple_command
%type <redirval> redir
%type <shtokenval> shtoken
%type <pipelineval> pipeline
%type <listkindval> list_sep list_term

%type <wordval> WORD
%type <numval> DIGIT_REDIR
%type <listkindval> SEMI AMPR DISJ CONJ

%start squash

%%

squash: %empty
      | SEMI				{ }
      | list				{ walk_tree($1); }
      ;

list: list list_term			{ $1->term = $2; }
    | list list_sep pipeline		{ $1->sep = $2; ast_pipeline_apppend($1->commands, $3); $1->ncommands++; }
    | pipeline				{ $$ = new_ast_list(LIST_Head, $1); }
    ;

list_term: %empty
	 | AMPR				{ $$ = $1; }
	 | SEMI				{ $$ = $1; }
	 ;

list_sep: DISJ				{ $$ = $1; }
	| CONJ				{ $$ = $1; }
	;

pipeline: pipeline PIPE simple_command  { ast_simple_command_append($1->commands, $3); $1->ncommands++; }
	| simple_command		{ $$ = new_ast_pipeline($1); }
	;

simple_command: simple_command shtoken	{ ast_shtoken_append($1->argv, $2); $1->nargs++; }
     	      | shtoken			{ $$ = new_ast_simple_command($1); }
	      ;

shtoken: WORD				{ $$ = new_ast_shtoken(SHTOKEN_Word, $1); }
       | redir				{ $$ = new_ast_shtoken(SHTOKEN_Redir, $1); }
       ;

redir: DIGIT_REDIR LANGLE WORD		{ $$ = new_ast_redir(REDIR_Out, $3); $$->fno = $1; }
     | DIGIT_REDIR RANGLE WORD		{ $$ = new_ast_redir(REDIR_In, $3); $$->fno = $1; }
     | DIGIT_REDIR DUPIN WORD		{ $$ = new_ast_redir(REDIR_DupIn, $3); $$->fno = $1; }
     | DIGIT_REDIR DUPOUT WORD		{ $$ = new_ast_redir(REDIR_DupOut, $3); $$->fno = $1; }
     | DIGIT_REDIR HEREDOC WORD		{ $$ = new_ast_redir(REDIR_HereDoc, $3); $$->fno = $1; }
     | DIGIT_REDIR HERESTR WORD		{ $$ = new_ast_redir(REDIR_HereStr, $3); $$->fno = $1; }
     | DIGIT_REDIR NCLBR WORD		{ $$ = new_ast_redir(REDIR_NoClobber, $3); $$->fno = $1; }
     | DIGIT_REDIR APPEND WORD		{ $$ = new_ast_redir(REDIR_Append, $3); $$->fno = $1; }
     | APPEND WORD              { $$ = new_ast_redir(REDIR_Append, $2); }
     | LANGLE WORD		{ $$ = new_ast_redir(REDIR_Out, $2);    }
     | RANGLE WORD              { $$ = new_ast_redir(REDIR_In, $2);     }
     | NCLBR WORD		{ $$ = new_ast_redir(REDIR_NoClobber, $2); }
     | HEREDOC WORD		{ $$ = new_ast_redir(REDIR_HereDoc, $2);  }
     | HERESTR WORD		{ $$ = new_ast_redir(REDIR_HereStr, $2);  }
     | DUPIN WORD		{ $$ = new_ast_redir(REDIR_DupIn, $2);   }
     | DUPOUT WORD		{ $$ = new_ast_redir(REDIR_DupOut, $2);  }
     ;


%%

void yyerror(const char *msg) {
  fprintf(stderr, "Parsing error occurred:\n");
  fprintf(stderr, "%s\n", msg);
}

void walk_simple_command(ASTSimpleCommand *cmd) {
  ASTShtoken *tmp = cmd->argv;
  while (tmp) {
    if (tmp->kind == SHTOKEN_Word)
      printf("%s\n---=\n", tmp->v_word->buffer);
    else if (tmp->kind == SHTOKEN_Redir) {
      printf("%s\n", tmp->v_redir->subj->buffer);
      printf("%i\n---=\n", tmp->v_redir->fno);
    }
    tmp = tmp->next;
  }
}

void walk_tree(ASTPipeline *pipeline) {
  ASTSimpleCommand *simple_cmd = pipeline->commands;
  while (simple_cmd) {
     walk_simple_command(simple_cmd);
     simple_cmd = simple_cmd->next;
  }
}
