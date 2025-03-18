#include "memory.h"

#include <stdio.h>
#include <string.h>

Maru_Arena *Maru_Arena_init(size_t size)
{
  Maru_Arena *arena = malloc(sizeof(Maru_Arena));

  if (arena == NULL)
    error("failed to allocate memory for Maru_Arena.");  

  arena->size = size;
  arena->index = 0;
  arena->ptr = malloc(size);
  memset(arena->ptr, 0, size);

  if (arena->ptr == NULL)
    error("failed to allocate memory for Maru_Arena.");

  return arena;
}

void *Maru_Arena_alloc(Maru_Arena *arena, size_t size)
{
  if (arena->index + size > arena->size)
    error("Maru_Arena is out of memory.");

  void *ptr = arena->ptr + arena->index;
  arena->index += size + 1;
  return ptr;
}

void Maru_Arena_free(Maru_Arena *arena)
{
  free(arena->ptr);
  free(arena);
}

Maru_String Maru_String_from_cstr(Maru_Arena *arena, const char *cstr)
{
  size_t size = strlen(cstr);

  Maru_String string = { 0 };
  string.cstr = Maru_Arena_alloc(arena, size+1);
  string.size = size;
  strncpy(string.cstr, cstr, size);
  
  return string;
}

Maru_String Maru_read_all(Maru_Arena *arena, Maru_String filename)
{
  Maru_String string = { 0 };
  FILE *file = fopen(filename.cstr, "r");

  if (file == NULL)
    error("failed to open file: '%s'", filename.cstr);

  fseek(file, 0, SEEK_END);
  string.size = ftell(file);
  rewind(file);
  string.cstr = Maru_Arena_alloc(arena, string.size);
  fread(string.cstr, string.size, 1, file);
  fclose(file);

  return string;
}
