#ifndef MEMORY_H
#define MEMORY_H

typedef struct GCObject GCObject;
typedef struct GCHeap GCHeap;

void gc_shutdown(void);
void gc_collect(void);
void gc_sweep(void);
void gc_mark(void);
void gc_free(void *memory);
void *gc_decref(void *memory);
void *gc_incref(void *memory);
void gc_realloc(void *memory,size_t new_size);
void *gc_alloc(size_t size);
GCObject *new_gc_object(GCObject **objects);
void gc_init(void);

#endif
