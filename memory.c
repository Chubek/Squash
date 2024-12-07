#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "memory.h"

#define ALIGNMENT 8
#define DEFAULT_HEAP_SIZE 8096

typedef struct GCObject {
  void *memory;
  bool marked;
  size_t size;
  size_t refs;
  struct GCObject *next;
} GCObject;

static struct GCHeap {
  GCObject *objects;
  size_t num_objects;
} *GCHeap = NULL;

void gc_init(void) {
  heap = malloc(sizeof(GCHeap));
  &heap->objects = NULL;
  heap->num_objects = 0;
}

GCObject *new_gc_object(GCObject **objects) {
  GCObject *obj = malloc(sizeof(GCObject));
  obj->memory = NULL;
  obj->marked = false;
  obj->size = 0;
  obj->refs = 0;
  obj->next = objects;
  *objects = obj;
  return obj;
}

void *gc_alloc(size_t size) {
  GCObject *obj = new_gc_object(&heap->objects);
  obj->memory = calloc(1, size);

  if (obj->memory == NULL) {
    fprintf(stderr, "Allocation error\n");
    return NULL;
  }

  obj->size = size;
  obj->refs = 1;
  heap->num_objects++;
  return obj->memory;
}

void gc_realloc(void *memory, size_t new_size) {
  GCObject **objects = &heap->objects;
  while (*objects) {
    if ((*objects)->memory == memory) {
      if ((*objects)->size > new_size) {
        fprintf(stderr, "Reallocation error: Wrong size\n");
        return NULL;
      }

      void *nptr = realloc((*objects)->memory, new_size);
      if (nptr == NULL) {
        fprintf(stderr, "Reallocation error\n");
        return NULL;
      }

      (*objects)->memory = nptr;
      (*objects)->size = new_size;

      return (*objects)->memory;
    }
    *objects = (*objects)->next;
  }

  return NULL;
}

void *gc_incref(void *memory) {
  GCOjbect **objects = &heap->objects;
  while (*objects) {
    if ((*objects)->memory == memory) {
      (*objects)->refs++;
      return (*objects)->memory;
    }
    *objects = (*objects)->next;
  }
}

void *gc_decref(void *memory) {
  GCObject **objects = &heap->objects;
  while (*objects) {
    if ((*objects)->memory == memory) {
      (*objects)->refs--;
      return (*objects)->memory;
    }
    *objects = (*objects)->next;
  }
}

void gc_free(void *memory) {
  GCObject **objects = &heap->objects;
  while (*objects) {
    if ((*objects)->memory == memory) {
      (*objects)->marked = true;
      free((*objects)->memory);
      return;
    }
    *objects = (*objects)->next;
  }
}

void gc_mark(void) {
  GCObject **objects = &heap->objects;
  while (*objects) {
    if ((*objects)->refs == 0)
      (*objects)->marked = true;
    *objects = (*objects)->next;
  }
}

void gc_sweep(void) {
  GCObject **objects = &heap->objects;
  while (*objects) {
    if ((*objects)->marked) {
      GCObject *to_free = *objects;
      *objects = to_free->next;
      free(to_free->memory);
      free(to_free);
    }

    *objects = (*objects)->next;
  }
}

void gc_collect(void) {
  gc_mark();
  gc_sweep();
}

void gc_shutdown(void) {
  gc_collect();
  free(heap);
}
