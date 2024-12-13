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
typedef struct ASTList ASTList;
typedef struct ASTCommand ASTCommand;

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
  ASTSimpleCommand *commands;
  size_t ncommands;
  ASTPipeline *next;
};

struct ASTList {
  enum ListKind {
    LIST_Head,
    LIST_And,
    LIST_Or,
    LIST_Semi,
    LIST_Amper,
  } sep;
  
  enum ListKind term;

  ASTPipeline *commands;
  size_t ncommands;
};

struct ASTCommand {
  enum CommandKind {
    COMMAND_Simple,
    COMMAND_Pipeline,
    COMMAND_List,
  } kind;

  union {
    ASTSimpleCommand *v_simplecmd;
    ASTPipeline *v_pipeline;
    ASTList *v_list;
  };

  ASTCommand *next;
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
ASTCommand *new_ast_command(enum CommandKind kind, void *new_cmd);
void ast_command_append(ASTCommand *head, ASTCommand *new_command);
void delete_ast_command_chain(ASTCommand *head);
void delete_ast_command(ASTCommand *cmd);
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
ASTList *new_ast_list(enum ListKind kind, ASTPipeline *head);
void delete_ast_list(ASTList *list);

#endif
