#ifndef ABSYN_H
#define ABSYN_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct ASTWord {
  uint8_t *buffer;
  size_t length;
  struct ASTWord *next;
} ASTWord;

typedef struct ASTRedir {
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
} ASTRedir;

typedef struct ASTShtoken {
  enum ShtokenKind {
    SHTOKEN_Word,
    SHTOKEN_Redir,
  } kind;

  union {
    ASTWord *v_word;
    ASTRedir *v_redir;
  };

  struct ASTShtoken *next;
} ASTShtoken;

typedef struct ASTSimpleCommand {
  ASTShtoken *argv;
  size_t nargs;
  ASTRedir *redir;
  struct ASTSimpleCommand *next;
} ASTSimpleCommand;

typedef struct ASTPipeline {
  ASTSimpleCommand *commands;
  size_t ncommands;
} ASTPipeline;

typedef struct ASTCommand {
  enum CommandKind {
    COMMAND_Simple,
    COMMAND_Pipeline,
  } kind;

  union {
    ASTSimpleCommand *v_simplecmd;
    ASTPipeline *v_pipeline;
  };
} ASTCommand;

ASTWord *new_ast_word(uint8_t *buffer, size_t length);
void delete_ast_word(ASTWord *word);
ASTWord *ast_digit_to_word(long digit);
void ast_word_append(ASTWord *word, ASTWord *new_word);
void delete_ast_word_chain(ASTWord *head);
ASTSimpleCommand *new_ast_simple_command(ASTShtoken *argv0);
void ast_simple_command_append(ASTSimpleCommand *head, ASTSimpleCommand *new_command);
void delete_ast_simple_command(ASTSimpleCommand *simplecmd);
void delete_ast_simple_command_chain(ASTSimpleCommand *head);
ASTCommand *new_ast_command(enum CommandKind kind, void *new_cmd);
void delete_ast_command(ASTCommand *cmd);
ASTRedir *new_ast_redir(enum RedirKind kind, ASTWord *subj);
void delete_ast_redir(ASTRedir *redir);
ASTShtoken *new_ast_shtoken(enum ShtokenKind kind, void *new_shtoken);
void ast_shtoken_append(ASTShtoken *shtoken, ASTShtoken *new_shtoken);
void delete_ast_shtoken(ASTShtoken *shtoken);
void delete_ast_shtoken_chain(ASTShtoken *head);
ASTPipeline *new_ast_pipeline(ASTSimpleCommand *head);
void delete_ast_pipeline(ASTPipeline *pipeline);

#endif
