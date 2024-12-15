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
  gc_incref(word);
  word->buffer = gc_strndup(buffer, length);
  word->length = length;
  word->next = NULL;
  return word;
}

ASTWord *new_ast_word_blank(void) {
  ASTWord *word = gc_alloc(sizeof(ASTWord));
  gc_incref(word);
  word->buffer = gc_alloc(1);
  gc_incref(word->buffer);
  word->length = 0;
  return word;
}

bool ast_word_compare_string(ASTWord *word, const uint8_t *against) {
  if (!strncmp(word->buffer, against, word->length))
    return true;
  return false;
}

bool ast_word_compare_word(ASTWord *word, ASTWord *against) {
  return ast_word_compare_string(word, against->buffer);
}

void ast_word_append_char(ASTWord *word, uint8_t ch) {
  word->buffer = gc_realloc(word->buffer, ++word->length);
  word->buffer[word->length - 1] = ch;
}

void delete_ast_word(ASTWord *word) {
  gc_free((void *)word->buffer);
  gc_decref(word);
}

void delete_ast_word_chain(ASTWord *head) {
  ASTWord *tmp = head;
  while (tmp) {
    ASTWord *to_free = tmp;
    tmp = tmp->next;
    delete_ast_word(to_free);
  }
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

ASTParam *new_ast_param(enum ParamKind kind, void *hook) {
  ASTParam *param = gc_alloc(sizeof(ASTParam));
  gc_incref(param);
  param->kind = kind;

  if (kind == PARAM_Positional)
    param->v_positional = *((int *)hook);
  else if (kind == PARAM_Special)
    param->v_special = *((char *)hook);
  else if (kind == PARAM_ShellVariable)
    param->v_variable = gc_incref(hook);

  return param;
}

void delete_ast_param(ASTParam *param) {
  if (param->kind == PARAM_ShellVariable)
    gc_decref(param->v_varaible);
  gc_decref(param);
}

ASTWordExpn *new_ast_wordexpn(enum WordExpnKind kind, void *hook) {
  ASTWordExpn *wordexpn = gc_alloc(sizeof(ASTWordExpn));
  gc_incref(wordexpn);
  wordexpn->kind = kind;
  wordexpn->next = NULL;

  if (kind == WEXPN_ParamExpn)
    wordexpn->v_paramexpn = hook;
  else if (kind == WEXPN_FieldSplit || kind == WEXPN_Pathname)
    wordexpn->v_word = hook;
  else if (kind == WEXPN_CommandSubst || kind == WEXPN_QuoteRemoval)
    wordexpn->v_sequence = hook;

  return wordexpn;
}

void ast_wordexpn_append(ASTWordExpn *head, ASTWordExpn *new_wordexpn) {
  ASTWordExpn *tmp = head;
  while (tmp->next != NULL)
    tmp = tmp->next;
  tmp->next = gc_incref(new_wordexpn);
}

void delete_ast_wordexpn(ASTWordExpn *wordexpn) {
  if (wordexpn->kind == WEXPN_ParamExpn)
    delete_ast_paramexpn(wordexpn->v_paramexpn);
  else if (wordexpn->kind == WEXPN_FieldSplit ||
           wordexpn->kind == WEXPN_Pathname)
    delete_ast_word(wordexpn->v_word);
  else if (wordexpn->kind == WEXPN_CommandSubst ||
           wordexpn->kind == WEXPN_QuoteRemoval)
    delete_ast_sequence(wordexpn->v_sequence);
  gc_decref(wordexpn);
}

void delete_ast_wordexpn_chain(ASTWordExpn *head) {
  ASTWordExpn *tmp = head;
  while (tmp) {
    ASTWordExpn *to_free = tmp;
    tmp = tmp->next;
    delete_ast_wordexpn(to_free);
  }
}

ASTParamExpn *new_ast_paramexpn(ASTParam *param, ASTWord *punct,
                                ASTSequence *seq) {
  ASTParamExpn *paramexpn = gc_alloc(sizeof(ASTParamExpn));
  gc_incref(paramexpn);
  paramexpn->param = gc_incref(param);
  paramexpn->punct = gc_incref(punct);
  paramexpn->seq = gc_incref(seq);
  return paramexpn;
}

ASTAssign *new_ast_assign(ASTWord *lhs, ASTSequence *rhs) {
  ASTAssign *assign = gc_alloc(sizeof(ASTAssign));
  gc_incref(assign);
  assign->lhs = gc_incref(lhs);
  assign->rhs = gc_incref(rhs);
  return assign;
}

void delete_ast_assign(ASTAssign *assign) {
  gc_decref(assign->lhs);
  gc_decref(assign->rhs);
  gc_decref(assign);
}

void delete_ast_paramexpn(ASTParamExpn *paramexpn) {
  gc_decref(paramexpn->param);
  gc_decref(paramexpn->punct);
  gc_decref(paramexpn->seq);
  gc_decref(paramexpn);
}

ASTSimpleCommand *new_ast_simple_command(ASTSequence *argv0) {
  ASTSimpleCommand *simplecmd = gc_alloc(sizeof(ASTSimpleCommand));
  gc_incref(simplecmd);
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
  delete_ast_sequence_chain(simplecmd->argv);
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

ASTRedir *new_ast_redir(enum RedirKind kind, ASTWord *subj) {
  ASTRedir *redir = gc_alloc(sizeof(ASTRedir));
  gc_incref(redir);
  redir->kind = kind;
  redir->subj = subj;
  redir->fno = -1;
  return redir;
}

void delete_ast_redir(ASTRedir *redir) { delete_ast_word(redir->subj); }

ASTSequence *new_ast_sequence(enum SequenceKind kind, void *new_sequence) {
  ASTSequence *sequence = gc_alloc(sizeof(ASTSequence));
  gc_incref(sequence);
  sequence->kind = kind;
  sequence->next = NULL;

  if (kind == SEQ_Word)
    sequence->v_word = new_sequence;
  else if (kind == SEQ_Redir)
    sequence->v_redir = new_sequence;
  else if (kind == SEQ_WordExpn || kind == SEQ_String)
    sequence->v_wordexpn = new_sequence;
  else if (kind == SEQ_Assign)
    sequence->v_assign = new_sequence;
  else if (kind == SEQ_Pattern)
    sequence->v_pattern = new_sequence;
}

void ast_sequence_append(ASTSequence *head, ASTSequence *new_sequence) {
  ASTSequence *tmp = head;
  while (tmp->next != NULL)
    tmp = tmp->next;
  tmp->next = new_sequence;
}

void delete_ast_sequence(ASTSequence *sequence) {
  if (sequence->kind == SEQ_Word)
    delete_ast_word(sequence->v_word);
  else if (sequence->kind == SEQ_Redir)
    delete_ast_redir(sequence->v_redir);
  else if (sequence->kind == SEQ_String || sequence->kind == SEQ_WordExpn)
    delete_ast_wordexpn(sequence->v_wordexpn);
  else if (sequence->kind == SEQ_Assign)
    delete_ast_assign(sequence->v_assign);
  else if (sequence->kind == SEQ_Pattern)
    delete_ast_pattern(sequence->v_pattern);
  gc_decref(sequence);
}

void delete_ast_sequence_chain(ASTSequence *head) {
  ASTSequence *tmp = head;
  while (tmp) {
    ASTSequence *to_free = tmp;
    tmp = tmp->next;
    delete_ast_sequence(to_free);
  }
}

ASTPipeline *new_ast_pipeline(ASTSimpleCommand *head) {
  ASTPipeline *pipeline = gc_alloc(sizeof(ASTPipeline));
  gc_incref(pipeline);
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
  gc_incref(list);
  list->commands = gc_incref(head);
  list->ncommands = 1;
}

void delete_ast_list(ASTList *list) {
  delete_ast_compound_chain(list->commands);
  gc_decref(list);
}

ASTCompound *new_ast_compound(enum CompoundKind kind, void *hook) {
  ASTCompound *compound = gc_alloc(sizeof(ASTCompound));
  gc_incref(compound);
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

void ast_compound_append(ASTCompound *head, ASTCompound *new_compound) {
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
  gc_incref(whileloop);
  whileloop->cond = gc_incref(cond);
  whileloop->body = gc_incref(body);
  return whileloop;
}

ASTUntilLoop *new_ast_untilloop(ASTList *cond, ASTList *body) {
  ASTUntilLoop *untilloop = gc_alloc(sizeof(ASTUntilLoop));
  gc_incref(untilloop);
  untilloop->cond = gc_incref(cond);
  untilloop->body = gc_incref(body);
  return untilloop;
}

ASTForLoop *new_ast_forloop(ASTWord *word, ASTWord *iter, ASTList *body) {
  ASTForLoop *forloop = gc_alloc(sizeof(ASTForLoop));
  gc_incref(forloop);
  forloop->name = gc_incref(word);
  forloop->iter = gc_incref(iter);
  forloop->body = gc_incref(body);
  return forloop;
}

ASTCaseCond *new_ast_casecond(ASTWord *discrim) {
  ASTCaseCond *casecond = gc_alloc(sizeof(ASTCaseCond));
  gc_incref(casecond);
  casecond->discrim = gc_incref(discrim);
  casecond->pairs = NULL;
  return casecond;
}

ASTIfCond *new_ast_ifcond(void) {
  ASTIfCond *ifcond = gc_alloc(sizeof(ASTIfCond));
  gc_incref(ifcond);
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
  delete_ast_word(forloop->name);
  delete_ast_word_chain(forloop->iter);
  delete_ast_list(forloop->body);
  gc_decref(forloop);
}

void delete_ast_casecond(ASTCaseCond *casecond) {
  delete_ast_word(casecond->discrim);
  struct ASTCasePair *tmp = casecond->pairs;
  while (tmp) {
    struct ASTCasePair *to_free = tmp;
    tmp = tmp->next;
    delete_ast_pattern_chain(to_free->clauses);
    delete_ast_list(to_free->body);
    gc_decref(to_free);
  }
  gc_decref(casecond);
}

void delete_ast_ifcond(ASTIfCond *ifcond) {
  struct ASTIfPair *tmp = ifcond->pairs;
  while (tmp) {
    struct ASTIfPair *to_free = tmp;
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
  struct ASTCasePair *tmp = casecond->pairs;
  while (tmp->next != NULL)
    tmp = tmp->next;
  tmp->next = gc_alloc(sizeof(struct ASTCasePair));
  tmp->next->clauses = gc_incref(clauses);
  tmp->next->body = gc_incref(body);
  tmp->next->next = NULL;
  gc_incref(tmp->next);
}

void ast_ifcond_pair_append(ASTIfCond *ifcond, ASTList *cond, ASTList *body) {
  struct ASTIfPair *tmp = ifcond->pairs;
  while (tmp->next != NULL)
    tmp = tmp->next;
  tmp->next = gc_alloc(sizeof(struct ASTIfPair));
  tmp->next->cond = gc_incref(cond);
  tmp->next->body = gc_incref(body);
  tmp->next->next = NULL;
  gc_incref(tmp->next);
}

ASTPattern *new_ast_pattern(enum PatternKind kind, ASTBracket *bracket) {
  ASTPattern *pattern = gc_alloc(sizeof(ASTPattern));
  gc_incref(pattern);
  pattern->kind = kind;
  pattern->bracket = bracket;
  pattern->next = NULL;
  return pattern;
}

void ast_pattern_append(ASTPattern *head, ASTPattern *new_pattern) {
  ASTPattern *tmp = head;
  while (tmp->next != NULL)
    tmp = tmp->next;
  tmp->next = gc_incref(new_pattern);
}

void delete_ast_pattern(ASTPattern *pattern) {
  gc_decref(pattern->bracket);
  gc_decref(pattern);
}

void delete_ast_pattern_chain(ASTPattern *head) {
  ASTPattern *tmp = head;
  while (tmp) {
    ASTPattern *to_free = tmp;
    tmp = tmp->next;
    delete_ast_pattern(tmp);
  }
}

ASTCharRange *new_ast_charrange(char start, char end) {
  ASTCharRange *charrange = gc_alloc(sizeof(ASTCharRange));
  gc_incref(charrange);
  charrange->start = start;
  charrange->end = end;
  charrange->next = NULL;
  return charrange;
}

void ast_charrange_append(ASTCharRange *head, ASTCharRange *new_charrange) {
  ASTCharRange *tmp = head;
  while (tmp->next != NULL)
    tmp = tmp->next;
  tmp->next = gc_incref(new_charrange);
}

void delete_ast_charrange(ASTCharRange *charrange) { gc_decref(charrange); }
void delete_ast_charrange_chain(ASTCharRange *head) {
  ASTCharRange *tmp = head;
  while (tmp) {
    ASTCharRange *to_free = tmp;
    tmp = tmp->next;
    delete_ast_charrange(to_free);
  }
}
ASTBracket *new_ast_bracket(ASTCharRange *ranges, bool negate) {
  ASTBracket *bracket = gc_alloc(sizeof(ASTBracket));
  gc_incref(bracket);
  bracket->ranges = ranges;
  bracket->negate = negate;
  return bracket;
}

void delete_ast_bracket(ASTBracket *bracket) {
  delete_ast_charrange_chain(bracket->ranges);
  gc_decref(bracket);
}
