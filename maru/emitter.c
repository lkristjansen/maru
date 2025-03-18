#include "emitter.h"
#include "parser.h"

#include <stdio.h>
#include <string.h>

static void emit_expr(FILE *file, Maru_Ast *ast);

static void emit_header(FILE *file)
{
  fprintf(file, "\t.text\n");
  fprintf(file, ".globl maru_entry\n");
  fprintf(file, "\t.type maru_entry, @function\n");
  fprintf(file, "maru_entry:\n");
}

static void emit_footer(FILE *file)
{
  fprintf(file, "\tret\n");
}

static void emit_immediate(FILE *file, Maru_Ast *ast)
{
  switch (ast->kind)
  {
  case Maru_AstKind_fixnum:
    fprintf(file, "\tmovq $%ld, %%rax\n", ast->value << 2);
    break;
  case Maru_AstKind_char:
    fprintf(file, "\tmovq $%ld, %%rax\n", (ast->value << 8) | 0xf);
    break;
  case Maru_AstKind_bool:
    fprintf(file, "\tmovq $%ld, %%rax\n", (ast->value << 7) | 0x1f);
    break;
  case Maru_AstKind_nil:
    fprintf(file, "\tmovq $0x2f, %%rax\n");
    break;
  default:
    error("unknown kind: %d", ast->kind);
  }
}

static void emit_is_zero(FILE *file, Maru_Ast *ast)
{
  emit_expr(file, ast->cons.car);
  fprintf(file, "\tcmpq $0, %%rax\n");
  fprintf(file, "\tmovq $0, %%rax\n");
  fprintf(file, "\tsete %%al\n");
  fprintf(file, "\tsalq $7, %%rax\n");
  fprintf(file, "\torq $0x1f, %%rax\n");
}

static void emit_is_null(FILE *file, Maru_Ast *ast)
{
  emit_expr(file, ast->cons.car);
  fprintf(file, "\tcmpq $0x2f, %%rax\n");
  fprintf(file, "\tmovq $0, %%rax\n");
  fprintf(file, "\tsete %%al\n");
  fprintf(file, "\tsalq $7, %%rax\n");
  fprintf(file, "\torq $0x1f, %%rax\n");
}

static void emit_add1(FILE *file, Maru_Ast *ast)
{
  emit_expr(file, ast->cons.car);
  fprintf(file, "\taddq $4, %%rax\n");
}

static void emit_sub1(FILE *file, Maru_Ast *ast)
{
  emit_expr(file, ast->cons.car);
  fprintf(file, "\tsubq $4, %%rax\n");
}

static void emit_char_to_integer(FILE *file, Maru_Ast *ast)
{
  emit_expr(file, ast->cons.car);
  fprintf(file, "\tshrq $6, %%rax\n");
}

static void emit_integer_to_char(FILE *file, Maru_Ast *ast)
{
  emit_expr(file, ast->cons.car);
  fprintf(file, "\tshlq $6, %%rax\n");
  fprintf(file, "\torq $0xf, %%rax\n");
}

static void emit_is_integer(FILE *file, Maru_Ast *ast)
{
  emit_expr(file, ast->cons.car);
  fprintf(file, "\tandq $3, %%rax\n");
  fprintf(file, "\tcmpq $0, %%rax\n");
  fprintf(file, "\tmovq $0, %%rax\n");
  fprintf(file, "\tsete %%al\n");
  fprintf(file, "\tsalq $7, %%rax\n");
  fprintf(file, "\torq $0x1f, %%rax\n");
}

static void emit_is_bool(FILE *file, Maru_Ast *ast)
{
  emit_expr(file, ast->cons.car);
  fprintf(file, "\tandq $0x7f, %%rax\n");
  fprintf(file, "\tcmpq $0x1f, %%rax\n");
  fprintf(file, "\tmovq $0, %%rax\n");
  fprintf(file, "\tsete %%al\n");
  fprintf(file, "\tsalq $7, %%rax\n");
  fprintf(file, "\torq $0x1f, %%rax\n");
}

static void emit_is_char(FILE *file, Maru_Ast *ast)
{
  emit_expr(file, ast->cons.car);
  fprintf(file, "\tandq $0xff, %%rax\n");
  fprintf(file, "\tcmpq $0xf, %%rax\n");
  fprintf(file, "\tmovq $0, %%rax\n");
  fprintf(file, "\tsete %%al\n");
  fprintf(file, "\tsalq $7, %%rax\n");
  fprintf(file, "\torq $0x1f, %%rax\n");
}

static void emit_expr(FILE *file, Maru_Ast *ast)
{
  if (Maru_Ast_is_immediate(ast))
  {
    emit_immediate(file, ast);
  }
  else if (ast->kind == Maru_AstKind_cons)
  {
    Maru_Ast *car = ast->cons.car;

    if (car->kind != Maru_AstKind_symbol)
      error("expected symbol got %d", car->kind);

    if (ast->cons.cdr->kind != Maru_AstKind_cons)
      error("expected argument.");

    if (strcmp(car->symbol.cstr, "zero?") == 0)
      emit_is_zero(file, ast->cons.cdr);
    else if (strcmp(car->symbol.cstr, "null?") == 0)
      emit_is_null(file, ast->cons.cdr);
    else if (strcmp(car->symbol.cstr, "add1") == 0)
      emit_add1(file, ast->cons.cdr);
    else if (strcmp(car->symbol.cstr, "sub1") == 0)
      emit_sub1(file, ast->cons.cdr);
    else if (strcmp(car->symbol.cstr, "char->integer") == 0)
      emit_char_to_integer(file, ast->cons.cdr);
    else if (strcmp(car->symbol.cstr, "integer->char") == 0)
      emit_integer_to_char(file, ast->cons.cdr);
    else if (strcmp(car->symbol.cstr, "integer?") == 0)
      emit_is_integer(file, ast->cons.cdr);
    else if (strcmp(car->symbol.cstr, "bool?") == 0)
      emit_is_bool(file, ast->cons.cdr);
    else if (strcmp(car->symbol.cstr, "char?") == 0)
      emit_is_char(file, ast->cons.cdr);
  }
}

void Maru_emit(FILE *file, Maru_Ast *ast)
{
  emit_header(file);
  emit_expr(file, ast);
  emit_footer(file);
}
