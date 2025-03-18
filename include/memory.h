#ifndef MARU_MEMORY_H
#define MARU_MEMORY_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define error(...)                                              \
  do {                                                          \
    char buf[256] = {0};                                        \
    sprintf(buf, __VA_ARGS__);                                  \
    fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, buf);   \
    exit(EXIT_FAILURE);                                         \
  } while(0);

typedef struct {
  void *ptr;
  size_t index;
  size_t size;
} Maru_Arena;

Maru_Arena *Maru_Arena_init(size_t size);

void *Maru_Arena_alloc(Maru_Arena *arena, size_t size);

void Maru_Arena_free(Maru_Arena *arena);

typedef struct {
  char *cstr;
  size_t size;
} Maru_String;

Maru_String Maru_String_from_cstr(Maru_Arena *arena, const char *cstr);

Maru_String Maru_read_all(Maru_Arena *arena, Maru_String filename);

#endif // MARU_MEMORY_H