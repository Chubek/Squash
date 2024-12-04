#include "parser.tab.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Arena Arena;

static struct Arena {
  size_t total;
  size_t used;
  struct Arena *next;
  char mem[];
} *main_arena = NULL;

Arena *new_region(size_t size) {
  Arena *arena = (Arena *)calloc(1, size);
  arena->total = size - (sizeof(uintptr_t) * 3);
  arena->used = 0;
  arena->next = NULL;
  return arena;
}

Arena *push_region(size_t size) {
  Arena *new_arena = new_region(size);
  new_arena->next = main_arena;
  main_arena = new_arena;
  return new_arena;
}

void *allocate_space(size_t size) {
  if ((main_arena->total - main_arena->used) < size)
    push_region();

  void *mem_space = (void *)main_arena->mem[main_arena->used + 1];
  main_arena->used += size + 1;

  return mem_space;
}

void free_regions(void) {
  Arena **current = &main_arena;
  while (*current) {
    Arena *to_free = *current;
    *current = to_free->next;
    free(to_free);
  }
}
