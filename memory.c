#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "memory.h"

#define ALIGNMENT 256
#define DEFAULT_HEAP_SIZE 8096

struct GCObject {
  void *memory;
  bool marked;
  size_t size;
  size_t refs;
  struct GCObject *next;
};

static struct GCHeap {
  GCObject *objects;
  size_t num_objects;
} *heap = NULL;

void gc_init(void) {
  heap = malloc(sizeof(GCHeap));
  heap->objects = NULL;
  heap->num_objects = 0;
}

GCObject *new_gc_object(void) {
  GCObject *obj = malloc(sizeof(GCObject));
  obj->memory = NULL;
  obj->marked = false;
  obj->size = 0;
  obj->refs = 0;
  obj->next = heap->objects;
  heap->objects = obj;
  return obj;
}

void *gc_alloc(size_t size) {
  GCObject *obj = new_gc_object();
  size_t aligned_size = (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
  obj->memory = calloc(1, aligned_size);

  if (obj->memory == NULL) {
    fprintf(stderr, "Allocation error\n");
    exit(EXIT_FAILURE);
  }

  obj->size = size;
  obj->refs = 0;
  heap->num_objects++;
  return obj->memory;
}

void *gc_realloc(void *memory, size_t new_size) {
  if (memory == NULL)
    return NULL;

  GCObject *objects = heap->objects;
  while (objects) {
    if (objects->memory == memory) {
      if (objects->size > new_size) {
        fprintf(stderr, "Reallocation error: Wrong size\n");
        exit(EXIT_FAILURE);
      }

      void *nptr = realloc(objects->memory, new_size);

      if (nptr == NULL) {
        fprintf(stderr, "Reallocation error\n");
        exit(EXIT_FAILURE);
      }

      objects->memory = nptr;
      objects->size = new_size;

      return objects->memory;
    }
    objects = objects->next;
  }

  return NULL;
}

void *gc_incref(void *memory) {
  if (memory == NULL)
    return NULL;

  GCObject *objects = heap->objects;
  while (objects) {
    if (objects->memory == memory) {
      objects->refs++;
      return objects->memory;
    }
    objects = objects->next;
  }
}

void *gc_decref(void *memory) {
  if (memory == NULL)
    return NULL;

  GCObject *objects = heap->objects;
  while (objects) {
    if (objects->memory == memory) {
      objects->refs--;
      return objects->memory;
    }
    objects = objects->next;
  }
}

void gc_free(void *memory) {
  if (memory == NULL)
    return;

  GCObject *objects = heap->objects;
  while (objects) {
    if (objects->memory == memory) {
      objects->marked = true;
      free(objects->memory);
      return;
    }
    objects = objects->next;
  }
}

void gc_mark(void) {
  GCObject *objects = heap->objects;
  while (objects) {
    if (objects->refs == 0 && objects->memory != NULL)
      objects->marked = true;
    objects = objects->next;
  }
}

void gc_sweep(void) {
  GCObject *objects = heap->objects;
  while (objects) {
    if (objects->marked && objects->memory != NULL) {
      GCObject *to_free = objects;
      objects = to_free->next;
      free(to_free->memory);
      free(to_free);
    }

    objects = objects->next;
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

uint8_t *gc_strndup(const uint8_t *str, size_t length) {
  uint8_t *mem = (uint8_t *)gc_alloc(length + 1);
  gc_incref(mem);
  uint8_t *dup = memmove(&mem[0], &str[0], length);
  return dup;
}
