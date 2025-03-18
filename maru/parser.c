#include "parser.h"
#include "memory.h"

#include <ctype.h>
#include <string.h>

static Maru_Ast *make_bool_ast(Maru_Arena *arena, int value)
{
  Maru_Ast *bool_ast = Maru_Arena_alloc(arena, sizeof(Maru_Ast));
  bool_ast->kind = Maru_AstKind_bool;
  bool_ast->value = value;
  return bool_ast;
}

static Maru_Ast *make_nil_ast(Maru_Arena *arena)
{
  Maru_Ast *nil_ast = Maru_Arena_alloc(arena, sizeof(Maru_Ast));
  nil_ast->kind = Maru_AstKind_nil;
  return nil_ast;
}

static Maru_Ast *make_char_ast(Maru_Arena *arena, char value)
{
  Maru_Ast *char_ast = Maru_Arena_alloc(arena, sizeof(Maru_Ast));
  char_ast->kind = Maru_AstKind_char;
  char_ast->value = value;
  return char_ast;
}

static Maru_Ast *make_fixnum_ast(Maru_Arena *arena, int64_t value)
{
  Maru_Ast *fixnum_ast = Maru_Arena_alloc(arena, sizeof(Maru_Ast));
  fixnum_ast->kind = Maru_AstKind_fixnum;
  fixnum_ast->value = value;
  return fixnum_ast;
}

static Maru_Ast *make_cons_ast(Maru_Arena *arena, Maru_Ast *car, Maru_Ast *cdr)
{
  Maru_Ast *cons_ast = Maru_Arena_alloc(arena, sizeof(Maru_Ast));
  cons_ast->kind = Maru_AstKind_cons;
  cons_ast->cons.car = car;
  cons_ast->cons.cdr = cdr;
  return cons_ast;
}

static Maru_Ast *make_symbol_ast(Maru_Arena *arena, Maru_String symbol)
{
  Maru_Ast *symbol_ast = Maru_Arena_alloc(arena, sizeof(Maru_Ast));
  symbol_ast->kind = Maru_AstKind_symbol;
  symbol_ast->symbol = symbol;
  return symbol_ast;
}

typedef struct Maru_Parser
{
  Maru_Arena *arena;
  Maru_String text;
  size_t index;
} Maru_Parser;

static int is_empty(Maru_Parser *parser)
{
  return parser->index >= parser->text.size;
}

static char current(Maru_Parser *parser)
{
  return parser->text.cstr[parser->index];
}

static void next(Maru_Parser *parser)
{
  parser->index += 1;
}

static char peek(Maru_Parser *parser)
{
  return parser->text.cstr[parser->index + 1];
}

static Maru_Ast *parse_expr(Maru_Parser *parser);

static Maru_Ast *parse_bool(Maru_Parser *parser)
{
  next(parser);

  if (is_empty(parser))
    error("unexpected eof.");

  char ch = current(parser);

  if (ch == 't')
    return make_bool_ast(parser->arena, 1);
  else if (ch == 'f')
    return make_bool_ast(parser->arena, 0);

  error("unexpected token '%c'.", ch);
}

static Maru_Ast *parse_cons(Maru_Parser *parser)
{
  next(parser);

  if (is_empty(parser))
    error("unexpected eof.");

  char ch = current(parser);

  // TODO(lkr): handle nil somewhere else?
  if (ch == ')')
    return make_nil_ast(parser->arena);

  Maru_Ast *car = parse_expr(parser);

  ch = current(parser);

  if (ch == ')')
  {
    Maru_Ast *nil_ast = make_nil_ast(parser->arena);
    return make_cons_ast(parser->arena, car, nil_ast);
  }

  Maru_Ast *cdr = parse_cons(parser);

  return make_cons_ast(parser->arena, car, cdr);
}

static Maru_Ast *parse_char(Maru_Parser *parser)
{
  next(parser);

  if (is_empty(parser))
    error("unexpected eof.");

  const char ch = current(parser);

  next(parser);

  if (is_empty(parser))
    error("unexpected eof.");

  if (current(parser) == '\'')
    return make_char_ast(parser->arena, ch);

  error("unexpected token.");
}

static Maru_Ast *parse_fixnum(Maru_Parser *parser)
{
  size_t start_index = parser->index;

  char ch = current(parser);

  if (ch == '-')
    next(parser);

  while (isdigit(current(parser)) != 0)
    next(parser);

  size_t size = parser->index - start_index;
  char *cstr = Maru_Arena_alloc(parser->arena, size + 1);
  strncpy(cstr, &parser->text.cstr[start_index], size);
  cstr[size] = '\0';
  int value = atoi(cstr);
  return make_fixnum_ast(parser->arena, value);
}

static int legal_symbol_char(char ch)
{
  return isalpha(ch) || ch == '?' || ch == '-' || ch == '>';
}

static Maru_Ast *parse_symbol(Maru_Parser *parser)
{
  size_t start_index = parser->index;
  char ch = current(parser);

  while (legal_symbol_char(ch) || isdigit(ch))
  {
    next(parser);
    ch = current(parser);
  }

  Maru_String symbol = {0};
  symbol.cstr = Maru_Arena_alloc(parser->arena, parser->index - start_index + 1);
  symbol.size = parser->index - start_index + 1;
  strncpy(symbol.cstr, parser->text.cstr + start_index, parser->index - start_index);
  symbol.cstr[parser->index - start_index] = '\0';
  return make_symbol_ast(parser->arena, symbol);
}

static Maru_Ast *parse_expr(Maru_Parser *parser)
{
  if (is_empty(parser))
    error("unexpected eof.");

  char ch = current(parser);

  while (isspace(ch))
  {
    next(parser);
    ch = current(parser);
  }

  if (ch == '#')
    return parse_bool(parser);
  else if (ch == '(')
    return parse_cons(parser);
  else if (ch == '\'')
    return parse_char(parser);
  else if (isdigit(ch) || (ch == '-' && isdigit(peek(parser))))
    return parse_fixnum(parser);
  else if (legal_symbol_char(ch))
    return parse_symbol(parser);

  error("unexpected token '%c'.", ch);
}

Maru_Ast *Maru_parse(Maru_Arena *arena, Maru_String input)
{
  Maru_Parser parser = {0};
  parser.arena = arena;
  parser.text = input;
  parser.index = 0;
  return parse_expr(&parser);
}

int Maru_Ast_is_immediate(Maru_Ast *ast)
{
  return ast->kind != Maru_AstKind_symbol && ast->kind != Maru_AstKind_cons;
}