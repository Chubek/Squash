#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "absyn.h"
#include "memory.h"

ASTWord *new_ast_word(uint8_t *buffer, size_t length) {
  ASTWord *word = gc_alloc(sizeof(ASTWord));
  word->buffer = gc_strndup((char *)buffer, length);
  word->length = length;
  word->next = NULL;
  return word;
}

void delete_ast_word(ASTWord *word) {
  gc_free((void*)word->buffer);
  gc_decref(word);
}

ASTWord *ast_digit_to_word(long digit) {
  switch (digit) {
  case 0:
    return new_ast_word("0", 1);
  case 1:
    return new_ast_word("1", 1);
  case 2:
    return new_ast_word("2", 1);
  case 3:
    return new_ast_word("3", 1);
  case 4:
    return new_ast_word("4", 1);
  case 5:
    return new_ast_word("5", 1);
  case 6:
    return new_ast_word("6", 1);
  case 7:
    return new_ast_word("7", 1);
  case 8:
    return new_ast_word("8", 1);
  case 9:
    return new_ast_word("9", 1);
  default:
    break;
  }
}

ASTSimpleCommand *new_ast_simple_command(ASTShtoken *argv0) {
  ASTSimpleCommand *simplecmd = gc_alloc(sizeof(ASTSimpleCommand));
  simplecmd->redir = NULL;
  simplecmd->argv = argv0;
  simplecmd->nargs = 1;
  return simplecmd;
}

void delete_ast_simple_command(ASTSimpleCommand *simplecmd) {
  if (simplecmd->redir != NULL)
    delete_ast_redir(simplecmd->redir);
  delete_ast_shtoken_chain(simplecmd->argv);
  gc_decref(simplecmd);
}

void ast_word_append(ASTWord *word, ASTWord *new_word) {
  ASTWord *tmp = word;
  while (tmp->next != NULL)
    tmp = tmp->next;
  tmp->next = gc_incref((void *)new_word);
}

void delete_ast_word_chain(ASTWord *head) {
  ASTWord *tmp = head;
  while (tmp) {
    ASTWord *to_free = tmp;
    tmp = tmp->next;
    delete_ast_word(to_free);
  }
}

ASTCommand *new_ast_command(enum CommandKind kind, void *new_cmd) {
  ASTCommand *cmd = gc_alloc(sizeof(ASTCommand));
  cmd->kind = kind;

  if (cmd->kind == COMMAND_Simple)
    cmd->v_simplecmd = new_cmd;
}

void delete_ast_command(ASTCommand *cmd) {
  if (cmd->kind == COMMAND_Simple)
    delete_ast_simple_command(cmd->v_simplecmd);
  gc_decref(cmd);
}

ASTRedir *new_ast_redir(enum RedirKind kind, ASTWord *subj) {
  ASTRedir *redir = gc_alloc(sizeof(ASTRedir));
  redir->kind = kind;
  redir->subj = subj;
  redir->fno = -1;
  return redir;
}

void delete_ast_redir(ASTRedir *redir) { delete_ast_word(redir->subj); }

ASTShtoken *new_ast_shtoken(enum ShtokenKind kind, void *new_shtoken) {
  ASTShtoken *shtoken = gc_alloc(sizeof(ASTShtoken));
  shtoken->kind = kind;
  shtoken->next = NULL;

  if (shtoken->kind == SHTOKEN_Word)
    shtoken->v_word = new_shtoken;
  else if (shtoken->kind == SHTOKEN_Redir)
    shtoken->v_redir = new_shtoken;
}

void ast_shtoken_append(ASTShtoken *head, ASTShtoken *new_shtoken) {
  ASTShtoken *tmp = head;
  while (tmp->next != NULL)
    tmp = tmp->next;
  tmp->next = new_shtoken;
}

void delete_ast_shtoken(ASTShtoken *shtoken) {
  if (shtoken->kind == SHTOKEN_Word)
    delete_ast_word(shtoken->v_word);
  else if (shtoken->kind == SHTOKEN_Redir)
    delete_ast_redir(shtoken->v_redir);
  gc_decref(shtoken);
}

void delete_ast_shtoken_chain(ASTShtoken *head) {
  ASTShtoken *tmp = head;
  while (tmp) {
    ASTShtoken *to_free = tmp;
    tmp = tmp->next;
    delete_ast_shtoken(to_free);
  }
}
