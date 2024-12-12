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
  ASTShtoken *shtokenval;
}

%token WORD ANCHORED_IDENTIFIER DOLLAR_IDENTIFIER INTEGER_NUMBER DIGIT
%token SEMI AMPR DISJ CONJ PIPE
%token LANGLE RANGLE APPEND DUPIN DUPOUT NCLBR HERESTR HEREDOC

%type <cmdval> command pipeline
%type <simplecmdval> simple_command
%type <redirval> redir
%type <wordval> word_chain redir_word
%type <shtokenval> shtoken

%type <wordval> WORD
%type <numval> DIGIT

%start squash

%%

squash: %empty
      | SEMI				{ }
      | simple_command SEMI		{ walk_tree($1); }
      ;

simple_command: simple_command shtoken	{ ast_shtoken_append($1->argv, $2); 
	      				  $1->nargs++; }
     	      | shtoken			{ $$ = new_ast_simple_command($1); }
	      ;

shtoken: WORD				{ $$ = new_ast_shtoken(SHTOKEN_Word, $1); }
       | redir				{ $$ = new_ast_shtoken(SHTOKEN_Redir, $1); }
       ;

redir: DIGIT LANGLE redir_word		{ $$ = new_ast_redir(REDIR_Out, $3); $$->fno = yylval.numval; }
     | DIGIT RANGLE redir_word		{ $$ = new_ast_redir(REDIR_In, $3); $$->fno = yylval.numval; }
     | DIGIT DUPIN redir_word		{ $$ = new_ast_redir(REDIR_DupIn, $3); $$->fno = yylval.numval; }
     | DIGIT DUPOUT redir_word		{ $$ = new_ast_redir(REDIR_DupOut, $3); $$->fno = yylval.numval; }
     | DIGIT HEREDOC redir_word		{ $$ = new_ast_redir(REDIR_HereDoc, $3); $$->fno = yylval.numval; }
     | DIGIT HERESTR redir_word		{ $$ = new_ast_redir(REDIR_HereStr, $3); $$->fno = yylval.numval; }
     | DIGIT NCLBR redir_word		{ $$ = new_ast_redir(REDIR_NoClobber, $3); $$->fno = yylval.numval; }
     | DIGIT APPEND redir_word		{ $$ = new_ast_redir(REDIR_Append, $3); $$->fno = yylval.numval; }
     | APPEND redir_word                { $$ = new_ast_redir(REDIR_Append, $2); }
     | LANGLE redir_word		{ $$ = new_ast_redir(REDIR_Out, $2);    }
     | RANGLE redir_word                { $$ = new_ast_redir(REDIR_In, $2);     }
     | NCLBR redir_word			{ $$ = new_ast_redir(REDIR_NoClobber, $2); }
     | HEREDOC redir_word		{ $$ = new_ast_redir(REDIR_HereDoc, $2);  }
     | HERESTR redir_word		{ $$ = new_ast_redir(REDIR_HereStr, $2);  }
     | DUPIN redir_word			{ $$ = new_ast_redir(REDIR_DupIn, $2);   }
     | DUPOUT redir_word		{ $$ = new_ast_redir(REDIR_DupOut, $2);  }
     ;

redir_word: WORD			{ $$ = $1; }
	  | DIGIT			{ $$ = ast_digit_to_word(yylval.numval); }
	  ;


%%

void yyerror(const char *msg) {
  fprintf(stderr, "Parsing error occurred:\n");
  fprintf(stderr, "%s\n", msg);
}

void walk_tree(ASTSimpleCommand *cmd) {
  ASTShtoken *tmp = cmd->argv;
  while (tmp) {
    if (tmp->kind == SHTOKEN_Word)
      printf("%s\n", tmp->v_word->buffer);
    else if (tmp->kind == SHTOKEN_Redir) {
      printf("%s\n", tmp->v_redir->subj->buffer);
      printf("%d\n", tmp->v_redir->fno);
    }
    tmp = tmp->next;
  }
}

