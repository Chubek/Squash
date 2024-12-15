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

static size_t current_range = 0;

void yyerror(const char *);

void walk_tree(ASTList*);

%}

%define parse.trace

%union {
  int numval;
  char paramval;
  char charval;
  ASTWord *wordval;
  ASTParam *astparamval;
  ASTWordExpn *wordexpnval;
  ASTPattern *patternval;
  ASTRedir *redirval;
  ASTCommand *cmdval;
  ASTSimpleCommand *simplecmdval;
  ASTPipeline *pipelineval;
  ASTSequence *sequenceval;
  ASTList *listval;
  ASTCompound *compoundval;
  ASTCaseCond *casecondval;
  ASTIfCond *ifcondval;
  ASTUntilLoop *untilloopval;
  ASTForLoop *forloopval;
  ASTWhileLoop *whileloopval;
  ASTCharRange *charrangeval;
  ASTBracket *bracketval;
  ASTFuncDef *funcdefval;
}

%token WORD FNNAME_IDENTIFIER DOLLAR_IDENTIFIER EXPN_IDENTIFIER EXPN_WORD EXPN_PUNCT
%token SEMI AMPR DISJ CONJ PIPE EQUAL
%token LANGLE RANGLE APPEND DUPIN DUPOUT NCLBR HERESTR HEREDOC
%token DIGIT_REDIR
%token SPECPARAM ARGNUM PARAM_IDENTIFIER
%token EXPN_START EXPN_END
%token KW_IF KW_ELSE KW_ELIF KW_THEN KW_FI
%token KW_WHILE KW_FOR KW_UNTIL
%token KW_CASE KW_ESAC
%token KW_IN KW_DO KW_DONE
%token FN_PARENS LPAREN RPAREN LCURLY RCURLY
%token BRACK_START BRACK_END BRACK_CHAR BRACK_DASH BRACK_BANG
%token TILDE BANG QMARK STAR 

%type <cmdval> command
%type <simplecmdval> simple_command
%type <redirval> redir
%type <sequenceval> sequence
%type <pipelineval> pipeline
%type <compoundval> compound_command
%type <listval> list
%type <charrangeval> char_range char_ranges
%type <bracketval> bracket
%type <patternval> pattern patterns
%type <funcdefval> func_def
%type <casecondval> case_cond
%type <ifcondval> if_cond
%type <untilloopval> until_loop
%type <forloopval> for_loop
%type <whileloopval> while_loop

%type <wordval> WORD ANCHORED_IDENTIFIER FNNAME_IDENTIFIER PARAM_IDENTIFIER EXPN_IDENTIFIER EXPN_WORD EXPN_PUNCT
%type <numval> DIGIT_REDIR
%type <charval> BRACK_CHAR

%start squash

%%

squash: %empty
      | SEMI				{ }
      | list				{ walk_tree($1); }
      ;

compound_command: compound_command SEMI { $1->term = LIST_Semi; }
		| compound_command AMPR { $1->term = LIST_Amper; }
		| list			{ $$ = new_ast_compound(COMPOUND_List, $1); }
		| pipeline		{ $$ = new_ast_compound(COMPOUND_Pipeline, $1); }
		| LCURLY compound_command RCURLY { $$ = new_ast_compound(COMPOUND_Group, $2); }
		| LPAREN compound_command SEMI RPAREN { $$ = new_ast_compound(COMPOUND_Subshell, $2); }
		;

list: list DISJ compound_command	{ $3->sep = LIST_Or; ast_compound_append($1->commands, $3); }
    | list CONJ compound_command	{ $3->sep = LIST_And; ast_compound_append($1->commands, $3); }
    | compound_command			{ $$ = new_ast_list($1); }
    ;

pipeline: pipeline PIPE simple_command  { ast_simple_command_append($1->commands, $3); $1->ncommands++; }
	| simple_command		{ $$ = new_ast_pipeline($1); }
	;

simple_command: simple_command sequence	{ ast_sequence_append($1->argv, $2); $1->nargs++; }
     	      | sequence		{ $$ = new_ast_simple_command($1); }
	      ;

sequence: WORD				{ $$ = new_ast_sequence(SEQ_Word, $1); }
        | redir				{ $$ = new_ast_sequence(SEQ_Redir, $1); }
	| patterns			{ $$ = new_ast_sequence(SEQ_Pattern, $1; }
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

patterns: patterns pattern	{ ast_pattern_append($1, $2); $$ = $1; }
	| pattern		{ $$ = $1; }
	;

pattern: STAR			{ $$ = new_ast_pattern(PATT_AnyString, NULL); }
       | QMARK			{ $$ = new_ast_pattern(PATT_AnyChar, NULL); }
       | bracket	        { $$ = new_ast_pattern(PATT_Bracket, $1); }
       ;

bracket: BRACK_START BRACK_BANG char_ranges BRACK_END { $$ = new_ast_bracket($3, true); }
       | BRACK_START char_ranges BRACK_END 	      { $$ = new_ast_bracket($2, false); }
       ;

char_ranges: char_ranges char_range	{ ast_charrange_append($1, $2); $$ = $1; }
	   | char_range			{ $$ = $1; }
	   ;

char_range: BRACK_CHAR BRACK_DASH BRACK_CHAR { $$ = new_ast_charrange($1, $3); }

%%

void yyerror(const char *msg) {
  fprintf(stderr, "Parsing error occurred:\n");
fprintf(stderr, "%s\n", msg);
}

void walk_simple_command(ASTSimpleCommand *cmd) {
  ASTSequence *tmp = cmd->argv;
  while (tmp) {
    if (tmp->kind == SEQ_Word)
      printf("%s\n---=\n", tmp->v_word->buffer);
    else if (tmp->kind == SEQ_Redir) {
      printf("%s\n", tmp->v_redir->subj->buffer);
      printf("%i\n---=\n", tmp->v_redir->fno);
    }
    tmp = tmp->next;
  }
}

void walk_pipeline(ASTPipeline *pipeline) {
  ASTSimpleCommand *simple_cmd = pipeline->commands;
  while (simple_cmd) {
     walk_simple_command(simple_cmd);
     simple_cmd = simple_cmd->next;
  }
}

void walk_tree(ASTList *list) {
  ASTPipeline *pipeline = list->commands;
  while (pipeline) {
    if (pipeline->sep == LIST_And)
      printf("-And-\n");
    else if (pipeline->sep == LIST_Or)
      printf("-Or-\n");
    if (pipeline->term == LIST_Semi)
      printf("-Semi-\n");
    else if (pipeline->term == LIST_Amper)
      printf("-Amper-\n");
    walk_pipeline(pipeline);
    pipeline = pipeline->next;
  }
}
