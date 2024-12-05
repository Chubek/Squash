#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

void *gc_memdup(const void *mem,size_t size);
char *gc_strndup(const char *str,size_t length);
void free_regions(void);
void *allocate_space(size_t size);
Arena *push_region(size_t size);
Arena *new_region(size_t size);

#endif
