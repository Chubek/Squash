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
  gc_free((void *)word->buffer);
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
  simplecmd->next = NULL;
  return simplecmd;
}

void ast_simple_command_append(ASTSimpleCommand *head,
                               ASTSimpleCommand *new_command) {
  ASTSimpleCommand *tmp = head;
  while (head->next != NULL)
    tmp = tmp->next;
  tmp->next = gc_incref(new_command);
}

void delete_ast_simple_command(ASTSimpleCommand *simplecmd) {
  if (simplecmd->redir != NULL)
    delete_ast_redir(simplecmd->redir);
  delete_ast_shtoken_chain(simplecmd->argv);
  gc_decref(simplecmd);
}

void delete_ast_simple_command_chain(ASTSimpleCommand *head) {
  ASTSimpleCommand *tmp = head;
  while (tmp) {
    ASTSimpleCommand *to_free = tmp;
    tmp = tmp->next;
    delete_ast_simple_command(to_free);
  }
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
    cmd->v_simplecmd = gc_incref(new_cmd);
  else if (cmd->kind == COMMAND_Pipeline)
    cmd->v_pipeline = gc_incref(new_cmd);
  else if (cmd->kind == COMMAND_List)
    cmd->v_list = gc_incref(new_cmd);
  else if (cmd->kind == COMMAND_Compound)
    cmd->v_compound = gc_incref(new_cmd);
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

ASTPipeline *new_ast_pipeline(ASTSimpleCommand *head) {
  ASTPipeline *pipeline = gc_alloc(sizeof(ASTPipeline));
  pipeline->commands = gc_incref(head);
  pipeline->ncommands = 1;
  pipeline->next = NULL;
  pipeline->sep = LIST_Head;
  pipeline->term = LIST_None;
  return pipeline;
}

void ast_pipeline_append(ASTPipeline *head, ASTPipeline *new_pipeline) {
  ASTPipeline *tmp = head;
  while (tmp->next != NULL)
    tmp = tmp->next;
  tmp->next = gc_incref(new_pipeline);
}

void delete_ast_pipeline(ASTPipeline *pipeline) {
  delete_ast_simple_command_chain(pipeline->commands);
  gc_decref(pipeline);
}

void delete_ast_pipeline_chain(ASTPipeline *head) {
  ASTPipeline *tmp = head;
  while (tmp) {
    ASTPipeline *to_free = tmp;
    tmp = tmp->next;
    delete_ast_pipeline(to_free);
  }
}

ASTList *new_ast_list(ASTPipeline *head) {
  ASTList *list = gc_alloc(sizeof(ASTList));
  list->commands = gc_incref(head);
  list->ncommands = 1;
}

void delete_ast_list(ASTList *list) {
  delete_ast_pipeline_chain(list->commands);
  gc_decref(list);
}

ASTCompound *new_ast_compound(enum CompoundKind kind, void *hook) {
  ASTCompound *compound = gc_alloc(sizeof(ASTCompound));
  compound->commands = head;
  compound->ncommands = 1;
  compound->next = NULL;

  if (kind == COMPOUND_List || kind == COMPOUND_Subshell ||
      kind == COMPOUND_Group)
    compound->v_list = gc_incref(hook);
  else if (kind == COMPOUND_ForLoop)
    compound->v_forloop = gc_incref(hook);
  else if (kind == COMPOUND_WhileLoop)
    compound->v_whileloop = gc_incref(hook);
  else if (kind == COMPOUND_UntilLoop)
    compound->v_untilloop = gc_incref(hook);
  else if (kind == COMPOUND_IfCond)
    compound->v_ifcond = gc_incref(hook);
  else if (kind == COMPOUND_CaseCond)
    compound->v_casecond = gc_incref(hook);
}

void ast_compound_append(ASTCompound *head, ASTCompound *next_compound) {
  ASTCompound *tmp = head;
  while (tmp->next != NULL)
    tmp = tmp->next;
  tmp->next = gc_incref(new_compound);
}

void delete_ast_compound(ASTCompound *compound) {
  if (compound->kind == COMPOUND_List || compound->kind == COMPOUND_Subshell ||
      compound->kind == COMPOUND_Group)
    delete_ast_list(compound->v_list);
  else if (compound->kind == COMPOUND_WhileLoop)
    delete_ast_whileloop(compound->v_whileloop);
  else if (compound->kind == COMPOUND_UntilLoop)
    delete_ast_untilloop(compound->v_untilloop);
  else if (compound->kind == COMPOUND_ForLoop)
    delete_ast_forloop(compound->v_forloop);
  else if (compound->kind == COMPOUND_CaseCond)
    delete_ast_casecond(compound->v_casecond);
  else if (compound->kind == COMPOUND_IfCond)
    delete_ast_ifcond(compound->v_ifcond);
  gc_decref(compound);
}

void delete_ast_compound_chain(ASTCompound *head) {
  ASTCompound *tmp = head;
  while (tmp) {
    ASTCompound *to_free = tmp;
    tmp = tmp->next;
    delete_ast_compound(to_free);
  }
}

ASTWhileLoop *new_ast_whileloop(ASTList *cond, ASTList *body) {
  ASTWhileLoop *whileloop = gc_alloc(sizeof(ASTWhileLoop));
  whileloop->cond = gc_incref(cond);
  whileloop->body = gc_incref(body);
  return whileloop;
}

ASTUntilLoop *new_ast_untilloop(ASTList *cond, ASTList *body) {
  ASTUntilLoop *untilloop = gc_alloc(sizeof(ASTUntilLoop));
  untilloop->cond = gc_incref(cond);
  untilloop->body = gc_incref(body);
  return untilloop;
}

ASTForLoop *new_ast_forloop(ASTWord *word, ASTWord *iter, ASTList *body) {
  ASTForLoop *forloop = gc_alloc(sizeof(ASTForLoop));
  forloop->word = gc_incref(word);
  forloop->iter = gc_incref(iter);
  forloop->body = gc_incref(body);
  return forloop;
}

ASTCaseCond *new_ast_casecond(ASTWord *discrim) {
  ASTCaseCond *casecond = gc_alloc(sizeof(ASTCaseCond));
  casecond->discrim = gc_incref(discrim);
  casecond->pairs = NULL;
  return casecond;
}

ASTIfCond *new_ast_ifcond(void) {
  ASTIfCond *ifcond = gc_alloc(sizeof(ASTIfCond));
  ifcond->pairs = NULL;
  ifcond->else_body = NULL;
  return ifcond;
}

void delete_ast_whileloop(ASTWhileLoop *whileloop) {
  delete_ast_list(whileloop->cond);
  delete_ast_list(whileloop->body);
  gc_decref(whileloop);
}

void delete_ast_untilloop(ASTUntilLoop *untilloop) {
  delete_ast_list(untilloop->cond);
  delete_ast_list(untilloop->body);
  gc_decref(untilloop);
}

void delete_ast_forloop(ASTForLoop *forloop) {
  delete_ast_word(forloop->discrim);
  delete_ast_word_chain(forloop->iter);
  delete_ast_list(forloop->body);
  gc_decref(forloop);
}

void delete_ast_casecond(ASTCaseCond *casecond) {
  delete_ast_word(casecond->discrim);
  struct CaseCondPair *tmp = casecond->pairs;
  while (tmp) {
    struct CaseCondPair *to_free = tmp;
    tmp = tmp->next;
    delete_ast_pattern_chain(to_free->clauses);
    delete_ast_list(to_free->body);
    gc_decref(to_free);
  }
  gc_decref(casecond);
}

void delete_ast_ifcond(ASTIfCond *ifcond) {
  struct IfCondPair *tmp = ifcond->pairs;
  while (tmp) {
    struct IfCondPair *to_free = tmp;
    tmp = tmp->next;
    delete_ast_list(to_free->cond);
    delete_ast_list(to_free->body);
    gc_decref(to_free);
  }
  if (ifcond->else_body != NULL)
    delete_ast_list(ifcond->else_body);
  gc_decref(ifcond);
}
void ast_casecond_pair_append(ASTCaseCond *casecond, ASTPattern *clauses,
                              ASTList *body) {
  struct CaseCondPair *tmp = casecond->pairs;
  while (tmp->next != NULL)
    tmp = tmp->next;
  tmp->next = gc_alloc(sizeof(struct CaseCondPair));
  tmp->next->clauses = gc_incref(clauses);
  tmp->next->body = gc_incref(body);
  tmp->next->next = NULL;
}

void ast_ifcond_pair_append(ASTIfCond *ifcond, ASTList *cond, ASTList *body) {
  struct IfCondPair *tmp = ifcond->pairs;
  while (tmp->next != NULL)
    tmp = tmp->next;
  tmp->next = gc_alloc(sizeof(struct IfCondPair));
  tmp->next->cond = gc_incref(cond);
  tmp->next->body = gc_incref(body);
  tmp->next->next = NULL;
}
