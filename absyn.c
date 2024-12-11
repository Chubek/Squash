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

ASTWord *ast_digit_to_word(long digit) {
  switch (digit) {
  case '0':
    return new_ast_word("0", 1);
  case '1':
    return new_ast_word("1", 1);
  case '2':
    return new_ast_word("2", 1);
  case '3':
    return new_ast_word("3", 1);
  case '4':
    return new_ast_word("4", 1);
  case '5':
    return new_ast_word("5", 1);
  case '6':
    return new_ast_word("6", 1);
  case '7':
    return new_ast_word("7", 1);
  case '8':
    return new_ast_word("8", 1);
  case '9':
    return new_ast_word("9", 1);
  default:
    break;
  }
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
  tmp->next = gc_incref((void *)new_word);
}

ASTCommand *new_ast_command(enum CommandKind kind, void *new_cmd) {
  ASTCommand *cmd = gc_alloc(sizeof(ASTCommand));
  cmd->kind = kind;

  if (cmd->kind == COMMAND_Simple)
    cmd->v_simplecmd = new_cmd;
}

ASTRedir *new_ast_redir(enum RedirKind kind, ASTWord *subj) {
  ASTRedir *redir = gc_alloc(sizeof(ASTRedir));
  redir->kind = kind;
  redir->subj = subj;
  redir->fno = -1;
  return redir;
}
