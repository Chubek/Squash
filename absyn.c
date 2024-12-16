#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "absyn.h"
#include "memory.h"

ASTBuffer *new_ast_buffer(uint8_t *buffer, size_t length) {
  ASTBuffer *buffer = gc_alloc(sizeof(ASTBuffer));
  gc_incref(buffer);
  buffer->buffer = gc_strndup(buffer, length);
  buffer->length = length;
  buffer->next = NULL;
  return buffer;
}

ASTBuffer *new_ast_buffer_blank(void) {
  ASTBuffer *buffer = gc_alloc(sizeof(ASTBuffer));
  gc_incref(buffer);
  buffer->buffer = gc_alloc(1);
  gc_incref(buffer->buffer);
  buffer->length = 0;
  return buffer;
}

bool ast_buffer_compare_string(ASTBuffer *buffer, const uint8_t *against) {
  if (!strncmp(buffer->buffer, against, buffer->length))
    return true;
  return false;
}

bool ast_buffer_compare_buffer(ASTBuffer *buffer, ASTBuffer *against) {
  return ast_buffer_compare_string(buffer, against->buffer);
}

void ast_buffer_append_char(ASTBuffer *buffer, uint8_t ch) {
  buffer->buffer = gc_realloc(buffer->buffer, ++buffer->length);
  buffer->buffer[buffer->length - 1] = ch;
}

void delete_ast_buffer(ASTBuffer *buffer) {
  gc_free((void *)buffer->buffer);
  gc_decref(buffer);
}

void delete_ast_buffer_chain(ASTBuffer *head) {
  ASTBuffer *tmp = head;
  while (tmp) {
    ASTBuffer *to_free = tmp;
    tmp = tmp->next;
    delete_ast_buffer(to_free);
  }
}

ASTBuffer *ast_digit_to_buffer(long digit) {
  switch (digit) {
  case 0:
    return new_ast_buffer("0", 1);
  case 1:
    return new_ast_buffer("1", 1);
  case 2:
    return new_ast_buffer("2", 1);
  case 3:
    return new_ast_buffer("3", 1);
  case 4:
    return new_ast_buffer("4", 1);
  case 5:
    return new_ast_buffer("5", 1);
  case 6:
    return new_ast_buffer("6", 1);
  case 7:
    return new_ast_buffer("7", 1);
  case 8:
    return new_ast_buffer("8", 1);
  case 9:
    return new_ast_buffer("9", 1);
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
    wordexpn->v_buffer = hook;
  else if (kind == WEXPN_CommandSubst || kind == WEXPN_QuoteRemoval)
    wordexpn->v_buffer = hook;

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
    delete_ast_buffer(wordexpn->v_buffer);
  else if (wordexpn->kind == WEXPN_CommandSubst ||
           wordexpn->kind == WEXPN_QuoteRemoval)
    delete_ast_word(wordexpn->v_buffer);
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

ASTParamExpn *new_ast_paramexpn(ASTParam *param, ASTBuffer *punct,
                                ASTWord *word) {
  ASTParamExpn *paramexpn = gc_alloc(sizeof(ASTParamExpn));
  gc_incref(paramexpn);
  paramexpn->param = gc_incref(param);
  paramexpn->punct = gc_incref(punct);
  paramexpn->word = gc_incref(word);
  return paramexpn;
}

ASTAssign *new_ast_assign(ASTBuffer *lhs, ASTWord *rhs) {
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
  gc_decref(paramexpn->word);
  gc_decref(paramexpn);
}

ASTSimpleCommand *new_ast_simple_command(ASTBuffer *prefix, ASTWord *argv0) {
  ASTSimpleCommand *simplecmd = gc_alloc(sizeof(ASTSimpleCommand));
  gc_incref(simplecmd);
  simplecmd->prefix = prefix;
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
  delete_ast_word_chain(simplecmd->argv);
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

void ast_buffer_append(ASTBuffer *buffer, ASTBuffer *new_buffer) {
  ASTBuffer *tmp = buffer;
  while (tmp->next != NULL)
    tmp = tmp->next;
  tmp->next = gc_incref((void *)new_buffer);
}

ASTRedir *new_ast_redir(enum RedirKind kind, ASTBuffer *subj) {
  ASTRedir *redir = gc_alloc(sizeof(ASTRedir));
  gc_incref(redir);
  redir->kind = kind;
  redir->subj = subj;
  redir->fno = -1;
  return redir;
}

void delete_ast_redir(ASTRedir *redir) { delete_ast_buffer(redir->subj); }

ASTWord *new_ast_word(enum WordKind kind, void *new_word) {
  ASTWord *word = gc_alloc(sizeof(ASTWord));
  gc_incref(word);
  word->kind = kind;
  word->next = NULL;

  if (kind == WORD_Buffer)
    word->v_buffer = new_word;
  else if (kind == WORD_Redir)
    word->v_redir = new_word;
  else if (kind == WORD_WordExpn || kind == WORD_String)
    word->v_wordexpn = new_word;
  else if (kind == WORD_Assign)
    word->v_assign = new_word;
  else if (kind == WORD_Pattern)
    word->v_pattern = new_word;
}

void ast_word_append(ASTWord *head, ASTWord *new_word) {
  ASTWord *tmp = head;
  while (tmp->next != NULL)
    tmp = tmp->next;
  tmp->next = new_word;
}

void delete_ast_word(ASTWord *word) {
  if (word->kind == WORD_Buffer)
    delete_ast_buffer(word->v_buffer);
  else if (word->kind == WORD_Redir)
    delete_ast_redir(word->v_redir);
  else if (word->kind == WORD_String || word->kind == WORD_WordExpn)
    delete_ast_wordexpn(word->v_wordexpn);
  else if (word->kind == WORD_Assign)
    delete_ast_assign(word->v_assign);
  else if (word->kind == WORD_Pattern)
    delete_ast_pattern(word->v_pattern);
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

ASTPipeline *new_ast_pipeline(ASTSimpleCommand *head) {
  ASTPipeline *pipeline = gc_alloc(sizeof(ASTPipeline));
  gc_incref(pipeline);
  pipeline->sep = SEP_None;
  pipeline->term = TERM_None;
  pipeline->commands = gc_incref(head);
  pipeline->ncommands = 1;
  pipeline->next = NULL;
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

void ast_list_append(ASTList *head, ASTList *new_list) {
  ASTList *tmp = head;
  while (tmp->next != NULL)
    tmp = tmp->next;
  tmp->next = gc_incref(new_list);
}

void delete_ast_list(ASTList *list) {
  delete_ast_compound_chain(list->commands);
  gc_decref(list);
}

void delete_ast_list_chain(ASTList *head) {
  ASTList *tmp = head;
  while (tmp) {
    ASTList *to_free = tmp;
    tmp = tmp->next;
    delete_ast_list(to_free);
  }
}

ASTCompound *new_ast_compound(enum CompoundKind kind, void *hook) {
  ASTCompound *compound = gc_alloc(sizeof(ASTCompound));
  gc_incref(compound);
  compound->next = NULL;
  compound->kind = kind;

  if (kind == COMPOUND_List)
    compound->v_list = gc_incref(hook);
  else if (kind == COMPOUND_Subshell || kind == COMPOUND_Group)
    compound->v_compoundlist = gc_incref(hook);
  else if (kind == COMPOUND_Pipeline)
    compound->v_pipeline = gc_incref(hook);
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

ASTWhileLoop *new_ast_whileloop(ASTCompoundList *cond, ASTCompoundList *body) {
  ASTWhileLoop *whileloop = gc_alloc(sizeof(ASTWhileLoop));
  gc_incref(whileloop);
  whileloop->cond = gc_incref(cond);
  whileloop->body = gc_incref(body);
  return whileloop;
}

ASTUntilLoop *new_ast_untilloop(ASTCompoundList *cond, ASTCompoundList *body) {
  ASTUntilLoop *untilloop = gc_alloc(sizeof(ASTUntilLoop));
  gc_incref(untilloop);
  untilloop->cond = gc_incref(cond);
  untilloop->body = gc_incref(body);
  return untilloop;
}

ASTForLoop *new_ast_forloop(ASTBuffer *buffer, ASTBuffer *iter,
                            ASTCompoundList *body) {
  ASTForLoop *forloop = gc_alloc(sizeof(ASTForLoop));
  gc_incref(forloop);
  forloop->name = gc_incref(buffer);
  forloop->iter = gc_incref(iter);
  forloop->body = gc_incref(body);
  return forloop;
}

ASTCaseCond *new_ast_casecond(ASTBuffer *discrim) {
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
  delete_ast_compound_list(whileloop->cond);
  delete_ast_compound_list(whileloop->body);
  gc_decref(whileloop);
}

void delete_ast_untilloop(ASTUntilLoop *untilloop) {
  delete_ast_compound_list(untilloop->cond);
  delete_ast_compound_list(untilloop->body);
  gc_decref(untilloop);
}

void delete_ast_forloop(ASTForLoop *forloop) {
  delete_ast_buffer(forloop->name);
  delete_ast_buffer_chain(forloop->iter);
  delete_ast_compound_list(forloop->body);
  gc_decref(forloop);
}

void delete_ast_casecond(ASTCaseCond *casecond) {
  delete_ast_buffer(casecond->discrim);
  struct ASTCasePair *tmp = casecond->pairs;
  while (tmp) {
    struct ASTCasePair *to_free = tmp;
    tmp = tmp->next;
    delete_ast_pattern_chain(to_free->clauses);
    delete_ast_compound_list(to_free->body);
    gc_decref(to_free);
  }
  gc_decref(casecond);
}

void delete_ast_ifcond(ASTIfCond *ifcond) {
  struct ASTIfPair *tmp = ifcond->pairs;
  while (tmp) {
    struct ASTIfPair *to_free = tmp;
    tmp = tmp->next;
    delete_ast_compound_list(to_free->cond);
    delete_ast_compound_list(to_free->body);
    gc_decref(to_free);
  }
  if (ifcond->else_body != NULL)
    delete_ast_compound_list(ifcond->else_body);
  gc_decref(ifcond);
}
void ast_casecond_pair_append(ASTCaseCond *casecond, ASTPattern *clauses,
                              ASTCompoundList *body) {
  struct ASTCasePair *tmp = casecond->pairs;
  while (tmp->next != NULL)
    tmp = tmp->next;
  tmp->next = gc_alloc(sizeof(struct ASTCasePair));
  tmp->next->clauses = gc_incref(clauses);
  tmp->next->body = gc_incref(body);
  tmp->next->next = NULL;
  gc_incref(tmp->next);
}

void ast_ifcond_pair_append(ASTIfCond *ifcond, ASTCompoundList *cond,
                            ASTCompoundList *body) {
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
  pattern->bracket = gc_incref(bracket);
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

ASTFuncDef *new_ast_funcdef(ASTBuffer *name, ASTCompound *body,
                            ASTRedir *redir) {
  ASTFuncDef *funcdef = gc_alloc(sizeof(ASTFuncDef));
  gc_incref(funcdef);
  funcdef->name = gc_incref(name);
  funcdef->body = gc_incref(body);
  funcdef->redir = gc_incref(redir);
  return funcdef;
}

void delete_ast_funcdef(ASTFuncDef *funcdef) {
  gc_decref(funcdef->name);
  gc_decref(funcdef->body);
  gc_decref(funcdef->redir);
}

ASTCompoundList *new_ast_compound_list(ASTList *head) {
  ASTCompoundList *compoundlist = gc_alloc(sizeof(ASTCompoundList));
  compoundlist->lists = head;
  compoundlist->nlists = 1;
  return compoundlist;
}

void delete_ast_compound_list(ASTCompoundList *compoundlist) {
  delete_ast_list_chain(compoundlist->lists);
  gc_decref(compoundlist);
}

ASTFactor *new_ast_factor(enum FactorKind kind, void *hook) {
  ASTFactor *factor = gc_alloc(sizeof(ASTFactor));
  factor->next = NULL;
  factor->kind = kind;

  if (kind == FACT_Number)
    factor->v_number = *((intmax_t *)hook);
  else if (kind == FACT_ArithExpr)
    factor->v_arithexpr = hook;

  return factor;
}

void ast_factor_append(ASTFactor *head, ASTFactor *new_factor) {
  ASTFactor *tmp = head;
  while (tmp->next != NULL)
    tmp = tmp->next;
  tmp->next = gc_incref(new_factor);
}

void delete_ast_factor(ASTFactor *factor) {
  if (factor->kind == FACT_ArithExpr)
    delete_ast_arithexpr(factor->v_arithexpr);
  gc_decref(factor);
}

void delete_ast_factor_chain(ASTFactor *head) {
  ASTFactor *tmp = head;
  while (tmp) {
    ASTFactor *to_free = tmp;
    tmp = tmp->next;
    delete_ast_factor(to_free);
  }
}

ASTArithExpr *new_ast_arithexpr(enum OperatorKind op, ASTFactor *left,
                                ASTFactor *right) {
  ASTArithExpr *arithexpr = gc_alloc(sizeof(ASTArithExpr));
  arithexpr->op = op;
  arithexpr->left = gc_incref(left);
  arithexpr->right = gc_incref(right);
  return arithexpr;
}

void delete_ast_arithexpr(ASTArithExpr *arithexpr) {
  delete_ast_factor_chain(arithexpr->left);
  delete_ast_factor_chain(arithexpr->right);
  gc_decref(arithexpr);
}
