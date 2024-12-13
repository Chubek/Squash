#ifndef ABSYN_H
#define ABSYN_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct ASTWord ASTWord;
typedef struct ASTParam ASTParam;
typedef struct ASTAssign ASTAssign;
typedef struct ASTWordExpn ASTWordExpn;
typedef struct ASTRedir ASTRedir;
typedef struct ASTSequence ASTSequence;
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

struct ASTParam {
  enum ParamKind {
    PARAM_Positional,
    PARAM_Special,
    PARAM_ShellVariable,
  } kind;

  union {
    int v_positional;
    char v_special;
    ASTWord *v_variable;
  };
};

struct ASTWordExpn {
  enum WordExpnKind {
    WEXPN_Tilde,
    WEXPN_ParamExpn,
    WEXPN_CommandSubst,
    WEXPN_FieldSplit,
    WEXPN_Pathname,
    WEXPN_QuoteRemoval,
  } kind;

  union {
    ASTWord *v_word;
    ASTParamExpn *v_paramexpn;
  };

  ASTWordExpn *next;
};

struct ASTParamExpn {
  ASTParam *param;
  ASTWord *punct;
  ASTSequence *seq;
};

struct ASTAssign {
  ASTWord *lhs;
  ASTSequence *rhs;
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

struct ASTSequence {
  enum SequenceKind {
    SEQ_Word,
    SEQ_Redir,
    SEQ_WordExpn,
    SEQ_Assign,
    SEQ_String,
  } kind;

  union {
    ASTWord *v_word;
    ASTRedir *v_redir;
    ASTWordExpn *v_wordexpn;
    ASTAssign *v_assign;
  };

  ASTSequence *next;
};

struct ASTSimpleCommand {
  ASTSequence *argv;
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

  ASTSimpleCommand *commands;
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
bool ast_word_compare(ASTWord *word, const uint8_t *discrim);
void delete_ast_word(ASTWord *word);
ASTParam *new_ast_param(enum ParamKind kind, void *hook);
void ast_param_append(ASTParam *head, ASTParam *new_param);
void delete_ast_param(ASTParam *param);
ASTWordExpn *new_ast_wordexpn(enum WordExpnKind kind, void *hook);
void ast_wordexpn_append(ASTWordExpn *head, ASTWordExpn *new_wordexpn);
void delete_ast_wordexpn(ASTWordExpn *wordexpn);
void delete_ast_wordexpn_chain(ASTWordExpn *head);
ASTParamExpn *new_ast_paramexpn(ASTParam *param, ASTWord *punct, ASTSequence *seq);
void delete_ast_paramexpn(ASTParamExpn *paramexpn);
ASTWord *ast_digit_to_word(long digit);
void ast_word_append(ASTWord *word, ASTWord *new_word);
void delete_ast_word_chain(ASTWord *head);
ASTSimpleCommand *new_ast_simple_command(ASTSequence *argv0);
void ast_simple_command_append(ASTSimpleCommand *head,
                               ASTSimpleCommand *new_command);
void delete_ast_simple_command(ASTSimpleCommand *simplecmd);
void delete_ast_simple_command_chain(ASTSimpleCommand *head);
ASTRedir *new_ast_redir(enum RedirKind kind, ASTWord *subj);
void delete_ast_redir(ASTRedir *redir);
ASTSequence *new_ast_sequence(enum SequenceKind kind, void *new_sequence);
void ast_sequence_append(ASTSequence *sequence, ASTSequence *new_sequence);
void delete_ast_sequence(ASTSequence *sequence);
void delete_ast_sequence_chain(ASTSequence *head);
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
void delete_ast_ifcond(ASTIfCond *ifcond);
void ast_casecond_pair_append(ASTCaseCond *casecond, ASTPattern *clause,
                              ASTList *body);
void ast_ifcond_pair_append(ASTIfCond *ifcond, ASTList *cond, ASTList *body);
ASTPattern *new_ast_pattern(void);
void ast_pattern_append(ASTPattern *head, ASTPattern *new_pattern);
void delete_ast_pattern(ASTPattern *pattern);
void delete_ast_pattern_chain(ASTPattern *head);
#endif
