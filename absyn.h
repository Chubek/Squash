#ifndef ABSYN_H
#define ABSYN_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct ASTBuffer ASTBuffer;
typedef struct ASTParam ASTParam;
typedef struct ASTWordExpn ASTWordExpn;
typedef struct ASTHereDoc ASTHereDoc;
typedef struct ASTRedir ASTRedir;
typedef struct ASTWord ASTWord;
typedef struct ASTSimpleCommand ASTSimpleCommand;
typedef struct ASTPipeline ASTPipeline;
typedef struct ASTCompound ASTCompound;
typedef struct ASTList ASTList;
typedef struct ASTCompoundList ASTCompoundList;
typedef struct ASTCommand ASTCommand;
typedef struct ASTForLoop ASTForLoop;
typedef struct ASTWhileLoop ASTWhileLoop;
typedef struct ASTUntilLoop ASTUntilLoop;
typedef struct ASTCaseCond ASTCaseCond;
typedef struct ASTIfCond ASTIfCond;
typedef struct ASTCharRange ASTCharRange;
typedef struct ASTBracket ASTBracket;
typedef struct ASTPattern ASTPattern;
typedef struct ASTFuncDef ASTFuncDef;
typedef struct ASTFactor ASTFactor;
typedef struct ASTArithExpr ASTArithExpr;

struct ASTBuffer {
  uint8_t *buffer;
  size_t length;
  ASTBuffer *next;
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
    ASTBuffer *v_variable;
  };
};

struct ASTCharRange {
  char start;
  char end;
  ASTCharRange *next;
};

struct ASTBracket {
  ASTCharRange *ranges;
  bool nranges;
};

struct ASTPattern {
  enum PatternKind {
    PATT_AnyString,
    PATT_AnyChar,
    PATT_Bracket,
  } kind;

  ASTBracket *bracket;
  ASTPattern *next;
};

struct ASTWordExpn {
  enum WordExpnKind {
    WEXPN_TildeExpn,
    WEXPN_ParamExpn,
    WEXPN_CommandSubst,
    WEXPN_ArithExpr,
    WEXPN_Pattern,
    WEXPN_Text,
  } kind;

  union {
    ASTBuffer *v_buffer;
    ASTParamExpn *v_paramexpn;
    ASTArithExpr *v_arithexpr;
    ASTPattern *v_pattern;
    ASTCompound *v_compound;
  };

  ASTWordExpn *next;
};

struct ASTParamExpn {
  ASTParam *param;
  ASTBuffer *punct;
  ASTWord *word;
};

struct ASTHereDoc {
  ASTBuffer *text;
  ASTBuffer *delim;
  ASTHereDoc *next;
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

  union {
     ASTBuffer *v_subj;
     ASTHereDoc *v_heredoc;
  };

  int fno;
};

struct ASTWord {
  enum WordKind {
    WORD_Buffer,
    WORD_Redir,
    WORD_WordExpn,
    WORD_QString,
    WORD_String,
  } kind;

  union {
    ASTBuffer *v_buffer;
    ASTRedir *v_redir;
    ASTWordExpn *v_wordexpn;
  };

  ASTWord *next;
};

struct ASTSimpleCommand {
  ASTBuffer *prefix; 
  ASTWord *argv;
  size_t nargs;
  ASTRedir *redir;
  ASTSimpleCommand *next;
};

struct ASTPipeline {
  enum ListKind {
    SEP_Head,
    SEP_And,
    SEP_Or,
    SEP_None,
  } sep;

  enum TermKind {
    TERM_Semi,
    TERM_Amper,
    TERM_None,
  } term;

  ASTSimpleCommand *commands;
  size_t ncommands;
  ASTPipeline *next;
};

struct ASTList {
  ASTCompound *commands;
  size_t ncommands;
  ASTList *next;
};

struct ASTCompoundList {
  ASTList *lists;
  size_t nlists;
};

struct ASTWhileLoop {
  ASTCompoundList *cond;
  ASTCompoundList *body;
};

struct ASTUntilLoop {
  ASTCompoundList *cond;
  ASTCompoundList *body;
};

struct ASTForLoop {
  ASTBuffer *name;
  ASTBuffer *iter;
  ASTCompoundList *body;
};

struct ASTCaseCond {
  ASTBuffer *discrim;
  struct ASTCasePair {
    ASTPattern *clauses;
    ASTCompoundList *body;
    struct ASTCasePair *next;
  } *pairs;
};

struct ASTIfCond {
  struct ASTIfPair {
    ASTCompoundList *cond;
    ASTCompoundList *body;
    struct ASTIfPair *next;
  } *pairs;
  ASTCompoundList *else_body;
};

struct ASTCompound {
  enum CompoundKind {
    COMPOUND_List,
    COMPOUND_SimpleCommand,
    COMPOUND_Pipeline,
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
    ASTSimpleCommand *v_simplecmd;
    ASTCompoundList *v_compoundlist;
    ASTPipeline *v_pipeline;
    ASTForLoop *v_forloop;
    ASTCaseCond *v_casecond;
    ASTIfCond *v_ifcond;
    ASTWhileLoop *v_whileloop;
    ASTUntilLoop *v_untilloop;
  };

  ASTCompound *next;
};

struct ASTFuncDef {
  ASTBuffer *name;
  ASTCompound *body;
  ASTRedir *redir;
};

struct ASTFactor {
  enum FactorKind {
    FACT_Number,
    FACT_ArithExpr,
  } kind;

  union {
    intmax_t v_number;
    ASTArithExpr *v_arithexpr;
  };

  ASTFactor *next;
};

struct ASTArithExpr {
  enum OperatorKind {
    OP_Add,
    OP_Sub,
    OP_Mul,
    OP_Div,
    OP_Mod,
    OP_Shr,
    OP_Shl,
  } op;

  ASTFactor *left;
  ASTFactor *right;
};

ASTBuffer *new_ast_buffer(uint8_t *buffer, size_t length);
ASTBuffer *new_ast_buffer_blank(void);
bool ast_buffer_compare_string(ASTBuffer *buffer, const uint8_t *against);
bool ast_buffer_compare_buffer(ASTBuffer *buffer, ASTBuffer *against);
void ast_buffer_append_char(ASTBuffer *buffer, uint8_t ch);
void ast_buffer_append_string(ASTBuffer *buffer, uint8_t *string, size_t length);
void delete_ast_buffer(ASTBuffer *buffer);
ASTParam *new_ast_param(enum ParamKind kind, void *hook);
void ast_param_append(ASTParam *head, ASTParam *new_param);
void delete_ast_param(ASTParam *param);
ASTWordExpn *new_ast_wordexpn(enum WordExpnKind kind, void *hook);
void ast_wordexpn_append(ASTWordExpn *head, ASTWordExpn *new_wordexpn);
void delete_ast_wordexpn(ASTWordExpn *wordexpn);
void delete_ast_wordexpn_chain(ASTWordExpn *head);
ASTParamExpn *new_ast_paramexpn(ASTParam *param, ASTBuffer *punct,
                                ASTWord *word);
void delete_ast_paramexpn(ASTParamExpn *paramexpn);
ASTBuffer *ast_digit_to_buffer(long digit);
void ast_buffer_append(ASTBuffer *buffer, ASTBuffer *new_buffer);
void delete_ast_buffer_chain(ASTBuffer *head);
ASTSimpleCommand *new_ast_simple_command(ASTBuffer *prefix, ASTWord *argv0);
void ast_simple_command_append(ASTSimpleCommand *head,
                               ASTSimpleCommand *new_command);
void delete_ast_simple_command(ASTSimpleCommand *simplecmd);
void delete_ast_simple_command_chain(ASTSimpleCommand *head);
ASTRedir *new_ast_redir(enum RedirKind kind, ASTBuffer *subj);
void delete_ast_redir(ASTRedir *redir);
ASTWord *new_ast_word(enum WordKind kind, void *new_word);
void ast_word_append(ASTWord *word, ASTWord *new_word);
void delete_ast_word(ASTWord *word);
void delete_ast_word_chain(ASTWord *head);
ASTPipeline *new_ast_pipeline(ASTSimpleCommand *head);
void ast_pipeline_append(ASTPipeline *head, ASTPipeline *new_pipeline);
void delete_ast_pipeline_chain(ASTPipeline *head);
void delete_ast_pipeline(ASTPipeline *pipeline);
ASTList *new_ast_list(ASTCompound *head);
void ast_list_append(ASTList *head, ASTList *new_list);
void delete_ast_list(ASTList *list);
void delete_ast_list_chain(ASTList *head);
ASTCompound *new_ast_compound(enum CompoundKind kind, void *hook);
void ast_compound_append(ASTCompound *head, ASTCompound *new_compound);
void delete_ast_compound(ASTCompound *compound);
void delete_ast_compound_chain(ASTCompound *head);
ASTWhileLoop *new_ast_whileloop(ASTCompoundList *cond, ASTCompoundList *body);
ASTUntilLoop *new_ast_untilloop(ASTCompoundList *cond, ASTCompoundList *body);
ASTForLoop *new_ast_forloop(ASTBuffer *buffer, ASTBuffer *iter,
                            ASTCompoundList *body);
ASTCaseCond *new_ast_casecond(ASTBuffer *discrim);
ASTIfCond *new_ast_ifcond(void);
void delete_ast_whileloop(ASTWhileLoop *whileloop);
void delete_ast_untilloop(ASTUntilLoop *untilloop);
void delete_ast_forloop(ASTForLoop *forloop);
void delete_ast_casecond(ASTCaseCond *casecond);
void delete_ast_ifcond(ASTIfCond *ifcond);
void ast_casecond_pair_append(ASTCaseCond *casecond, ASTPattern *clause,
                              ASTCompoundList *body);
void ast_ifcond_pair_append(ASTIfCond *ifcond, ASTCompoundList *cond,
                            ASTCompoundList *body);
ASTPattern *new_ast_pattern(enum PatternKind kind, ASTBracket *bracket);
void ast_pattern_append(ASTPattern *head, ASTPattern *new_pattern);
void delete_ast_pattern(ASTPattern *pattern);
void delete_ast_pattern_chain(ASTPattern *head);
ASTCharRange *new_ast_charrange(char start, char end);
void ast_charrange_append(ASTCharRange *head, ASTCharRange *new_charrange);
void delete_ast_charrange(ASTCharRange *charrange);
void delete_ast_charrange_chain(ASTCharRange *head);
ASTBracket *new_ast_bracket(ASTCharRange *ranges, bool negate);
void delete_ast_bracket(ASTBracket *bracket);
ASTFuncDef *new_ast_funcdef(ASTBuffer *name, ASTCompound *body,
                            ASTRedir *redir);
void delete_ast_funcdef(ASTFuncDef funcdef);
ASTCompoundList *new_ast_compound_list(ASTList *head);
void delete_ast_compound_list(ASTCompoundList *compoundlist);
ASTFactor *new_ast_factor(enum FactorKind kind, void *hook);
void ast_factor_append(ASTFactor *head, ASTFactor *new_factor);
void delete_ast_factor(ASTFactor *factor);
void delete_ast_factor_chain(ASTFactor *head);
ASTArithExpr *new_ast_arithexpr(enum OperatorKind op, ASTFactor *left,
                                ASTFactor *right);
void delete_ast_arithexpr(ASTArithExpr *arithexpr);
ASTHereDoc *new_ast_heredoc(ASTBuffer *text, ASTBuffer *delim);
void ast_heredoc_append(ASTHereDoc *head, ASTHereDoc *new_heredoc);
void delete_ast_heredoc(ASTHereDoc *heredoc);
void delete_ast_heredoc_chain(ASTHereDoc *head);
#endif
