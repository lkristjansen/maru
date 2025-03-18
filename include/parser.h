#ifndef MARU_PARSER_H
#define MARU_PARSER_H

#include "memory.h"

#include <stdint.h>

typedef enum {
  Maru_AstKind_fixnum,
  Maru_AstKind_char,
  Maru_AstKind_bool,
  Maru_AstKind_nil,
  Maru_AstKind_cons,
  Maru_AstKind_symbol
} Maru_AstKind;

typedef struct Maru_Ast {
  int kind;
  union {
    int64_t value;
    struct {
      struct Maru_Ast *car;
      struct Maru_Ast *cdr;
    } cons;
    Maru_String symbol;
  };
} Maru_Ast;

Maru_Ast* Maru_parse(Maru_Arena *arena, Maru_String input);

int Maru_Ast_is_immediate(Maru_Ast *ast);

#endif // MARU_PARSER_H
