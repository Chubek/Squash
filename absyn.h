#ifndef ABSYN_H
#define ABSYN_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct ASTWord ASTWord;
typedef struct ASTRedir ASTRedir;
typedef struct ASTShtoken ASTShtoken;
typedef struct ASTSimpleCommand ASTSimpleCommand;
typedef struct ASTPipeline ASTPipeline;
typedef struct ASTCompound ASTCompound;
typedef struct ASTList ASTList;
typedef struct ASTCommand ASTCommand;
typedef struct ASTForLoop ASTForLoop;
typedef struct ASTWhileLoop ASTWhileLoop;
typedef struct ASTUntilLoop ASTUntilLoop;
typedef struct ASTCaseCond ASTCaseCond;
typedef struct ASTIfCond ASTIfCond;
typedef struct ASTPattern ASTPattern;

struct ASTWord {
  uint8_t *buffer;
  size_t length;
  ASTWord *next;
};

struct ASTRedir {
  enum RedirKind {
    REDIR_In,
    REDIR_Out,
    REDIR_Append,
    REDIR_DupIn,
    REDIR_DupOut,
    REDIR_HereStr,
    REDIR_HereDoc,
    REDIR_RW,
    REDIR_NoClobber,
  } kind;

  int fno;
  ASTWord *subj;
};

struct ASTShtoken {
  enum ShtokenKind {
    SHTOKEN_Word,
    SHTOKEN_Redir,
  } kind;

  union {
    ASTWord *v_word;
    ASTRedir *v_redir;
  };

  ASTShtoken *next;
};

struct ASTSimpleCommand {
  ASTShtoken *argv;
  size_t nargs;
  ASTRedir *redir;
  ASTSimpleCommand *next;
};

struct ASTPipeline {
  enum ListKind {
    LIST_None,
    LIST_Head,
    LIST_And,
    LIST_Or,
    LIST_Semi,
    LIST_Amper,
    LIST_Newline,
  } sep;

  enum ListKind term;

  ASTCompound *commands;
  size_t ncommands;
  ASTPipeline *next;
};

struct ASTList {
  ASTCompound *commands;
  size_t ncommands;
};

struct ASTWhileLoop {
  ASTList *cond;
  ASTList *body;
};

struct ASTUntilLoop {
  ASTList *cond;
  ASTList *body;
};

struct ASTForLoop {
  ASTWord *name;
  ASTWord *iter;
  ASTList *body;
};

struct ASTCaseCond {
  ASTWord *discrim;
  struct ASTCasePair {
    ASTPattern *clauses;
    ASTList *body;
    struct ASTCasePair *next;
  } *pairs;
};

struct ASTIfCond {
  struct ASTIfPair {
    ASTList *cond;
    ASTList *body;
    struct ASTIfPair *next;
  } *pairs;
  ASTList *else_body;
};

struct ASTCompound {
  enum CompoundKind {
    COMPOUND_List,
    COMPOUND_Subshell,
    COMPOUND_Group,
    COMPOUND_ForLoop,
    COMPOUND_CaseCond,
    COMPOUND_IfCond,
    COMPOUND_WhileLoop,
    COMPOUND_UntilLoop,
  } kind;

  union {
    ASTList *v_list;
    ASTForLoop *v_forloop;
    ASTCaseCond *v_casecond;
    ASTIfCond *v_ifcond;
    ASTWhileLoop *v_whileloop;
    ASTUntilLoop *v_untilloop;
  };

  ASTCompound *next;
};

ASTWord *new_ast_word(uint8_t *buffer, size_t length);
void delete_ast_word(ASTWord *word);
ASTWord *ast_digit_to_word(long digit);
void ast_word_append(ASTWord *word, ASTWord *new_word);
void delete_ast_word_chain(ASTWord *head);
ASTSimpleCommand *new_ast_simple_command(ASTShtoken *argv0);
void ast_simple_command_append(ASTSimpleCommand *head,
                               ASTSimpleCommand *new_command);
void delete_ast_simple_command(ASTSimpleCommand *simplecmd);
void delete_ast_simple_command_chain(ASTSimpleCommand *head);
ASTRedir *new_ast_redir(enum RedirKind kind, ASTWord *subj);
void delete_ast_redir(ASTRedir *redir);
ASTShtoken *new_ast_shtoken(enum ShtokenKind kind, void *new_shtoken);
void ast_shtoken_append(ASTShtoken *shtoken, ASTShtoken *new_shtoken);
void delete_ast_shtoken(ASTShtoken *shtoken);
void delete_ast_shtoken_chain(ASTShtoken *head);
ASTPipeline *new_ast_pipeline(ASTSimpleCommand *head);
void ast_pipeline_append(ASTPipeline *head, ASTPipeline *new_pipeline);
void delete_ast_pipeline_chain(ASTPipeline *head);
void delete_ast_pipeline(ASTPipeline *pipeline);
ASTList *new_ast_list(ASTPipeline *head);
void delete_ast_list(ASTList *list);
ASTCompound *new_ast_compound(enum CompoundKind kind, void *hook);
void ast_compound_append(ASTCompound *head, ASTCompound *new_compound);
void delete_ast_compound(ASTCompound *compound);
void delete_ast_compound_chain(ASTCompound *head);
ASTWhileLoop *new_ast_whileloop(ASTList *cond, ASTList *body);
ASTUntilLoop *new_ast_untilloop(ASTList *cond, ASTList *body);
ASTForLoop *new_ast_forloop(ASTWord *word, ASTWord *iter, ASTList *body);
ASTCaseCond *new_ast_casecond(ASTWord *discrim);
ASTIfCond *new_ast_ifcond(void);
void delete_ast_whileloop(ASTWhileLoop *whileloop);
void delete_ast_untilloop(ASTUntilLoop *untilloop);
void delete_ast_forloop(ASTForLoop *forloop);
void delete_ast_casecond(ASTCaseCond *casecond);
void delete_ast_ifcond(ASTIfCOnd *ifcond);
void ast_casecond_pair_append(ASTCaseCond *casecond, ASTPattern *clause, ASTList *body);
void ast_ifcond_pair_append(ASTIfCond *ifcond, ASTList *cond, ASTList *body);
#endif
