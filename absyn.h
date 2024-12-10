#ifndef ABSYN_H
#define ABSYN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct ASTWord {
  const uint8_t *buffer;
  size_t length;
  struct ASTWord *next;
} ASTWord;

typedef struct ASTRedir {
  enum {
    REDIR_In,
    REDIR_Out,
    REDIR_Append,
  } kind;

  ASTWord *subj;
} ASTRedir;

typedef struct ASTSimpleCommnad {
  const ASTWord *argv;
  size_t nargs;
  ASTRedir *redir;
} ASTSimpleCommnad;

typedef struct ASTCommand {
  enum CommandKind {
    COMMNAD_Simple,
  } kind;

  union {
    ASTSimpleCommand *v_simplecmd;
  };
} ASTCommand;

ASTWord *new_ast_word(const uint8_t *buffer, size_t length);
void ast_word_append(ASTWord *word, ASTWord *new_word);
ASTSimpleCommand *new_ast_simple_command(ASTRedir *redir);
ASTCommand *new_ast_command(enum CommandKind kind, void *new_cmd);

#endif
