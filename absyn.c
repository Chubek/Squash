#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "absyn.h"
#include "memory.h"

ASTWord *new_ast_word(const uint8_t *buffer, size_t length) {
  ASTWord *word = gc_alloc(sizeof(ASTWord));
  word->buffer = gc_strndup((const char *)buffer, length);
  word->length = length;
  word->next = NULL;
  return word;
}

ASTSimpleCommand *new_ast_simple_command(const ASTWord *argv0) {
  ASTSimpleCommand *simplecmd = gc_alloc(sizeof(ASTSimpleCommand));
  simplecmd->redir = NULL;
  simplecmd->argv = argv0;
  simplecmd->nargs = 1;
  return simplecmd;
}

void ast_word_append(ASTWord *word, const ASTWord *new_word) {
  ASTWord *tmp = word;
  while (tmp->next != NULL)
    tmp = tmp->next;
  tmp->next = gc_incref((void*)new_word);
}

ASTCommand *new_ast_command(enum CommandKind kind, void *new_cmd) {
  ASTCommand *cmd = gc_alloc(sizeof(ASTCommand));
  cmd->kind = kind;

  if (cmd->kind == COMMAND_Simple)
    cmd->v_simplecmd = new_cmd;
}


