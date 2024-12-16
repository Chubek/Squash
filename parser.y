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
  ASTBuffer *bufferval;
  ASTParam *astparamval;
  ASTBufferExpn *bufferexpnval;
  ASTPattern *patternval;
  ASTRedir *redirval;
  ASTCommand *cmdval;
  ASTSimpleCommand *simplecmdval;
  ASTPipeline *pipelineval;
  ASTWord *wordval;
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
%type <wordval> word
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

%type <bufferval> WORD ANCHORED_IDENTIFIER FNNAME_IDENTIFIER PARAM_IDENTIFIER EXPN_IDENTIFIER EXPN_WORD EXPN_PUNCT
%type <numval> DIGIT_REDIR
%type <charval> BRACK_CHAR

%start squash

%%

squash: %empty
      | SEMI				{ }
      | list				{ walk_tree($1); }
      ;

compound_command: list					{ $$ = new_ast_compound(COMPOUND_List, $1); }
		| LPAREN compound_list RPAREN 		{ $$ = new_ast_compound(COMPOUND_Subshell, $2); }
		| LCURLY compound_list SEMI RCURLY 	{ $$ = new_ast_compound(COMPOUND_Group, $2); }
		| LCURLY compound_list NEWLINE RCURLY   { $$ = new_ast_compound(COMPOUND_Group, $2);  }
		;

compound_list: compound_list NEWLINE list	{ ast_list_append($1->lists, $2); $1->nlists++; }
	     | list				{ $$ = new_ast_compound_list($1); }
	     ;

list: list DISJ pipeline		{ $3->sep = SEP_Or; ast_compound_append($1->commands, $3); }
    | list CONJ pipeline		{ $3->sep = SEP_And; ast_compound_append($1->commands, $3); }
    | pipeline				{ $$ = new_ast_list($1); }
    ;

pipeline: pipeline SEMI			{ $1->term = TERM_Semi; }
	| pipeline AMPR			{ $1->term = TERM_Amper; }
	| pipeline PIPE simple_command  { ast_simple_command_append($1->commands, $3); $1->ncommands++; }
	| simple_command		{ $$ = new_ast_pipeline($1); }
	;

simple_command: simple_command word	{ ast_word_append($1->argv, $2); $1->nargs++; }
     	      | word		{ $$ = new_ast_simple_command($1); }
	      ;

word: WORD				{ $$ = new_ast_word(WORD_Buffer, $1); }
        | redir				{ $$ = new_ast_word(WORD_Redir, $1); }
	| patterns			{ $$ = new_ast_word(WORD_Pattern, $1; }
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
  ASTWord *tmp = cmd->argv;
  while (tmp) {
    if (tmp->kind == WORD_Buffer)
      printf("%s\n---=\n", tmp->v_buffer->buffer);
    else if (tmp->kind == WORD_Redir) {
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
    if (pipeline->sep == SEP_And)
      printf("-And-\n");
    else if (pipeline->sep == SEP_Or)
      printf("-Or-\n");
    if (pipeline->term == TERM_Semi)
      printf("-Semi-\n");
    else if (pipeline->term == TERM_Amper)
      printf("-Amper-\n");
    walk_pipeline(pipeline);
    pipeline = pipeline->next;
  }
}
