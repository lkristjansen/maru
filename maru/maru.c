#include "emitter.h"
#include "memory.h"
#include "parser.h"

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

static void dump_ast(FILE *file, Maru_Ast *ast)
{
  switch (ast->kind)
  {
    case Maru_AstKind_bool:
      fprintf(file, "#%c", ast->value ? 't' : 'f');
      break;
    case Maru_AstKind_char:
      fprintf(file, "'%c'", (char) ast->value);
      break;
    case Maru_AstKind_fixnum:
      fprintf(file, "%ld", ast->value);
      break;
    case Maru_AstKind_nil:
      fprintf(file, "()");
      break;
    case Maru_AstKind_cons:
      printf("(");
      dump_ast(file, ast->cons.car);
      printf(" . ");
      dump_ast(file, ast->cons.cdr);
      printf(")");
      break;
    case Maru_AstKind_symbol:
      printf("%s", ast->symbol.cstr);
      break;
    default:
      error("unknown ast kind: '%d'", ast->kind);
  }
}

int main(int argc, char** argv)
{
  int option;
  Maru_String outfile = {0};
  int do_dump_ast = 0;
  Maru_Arena *arena = Maru_Arena_init(1024 * 1024);

  while((option = getopt(argc, argv, "do:")) != -1)
  {
    switch (option)
    {
      case 'o':
        outfile = Maru_String_from_cstr(arena, optarg);
        break;
      case 'd':
        do_dump_ast = 1;
        break;
      default:
        fprintf(stderr, "Usage: %s [-o outfile | -d] <infile>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  if (optind >= argc)
  {
    fprintf(stderr, "Expected arguments after optional arguments.\n");
    exit(EXIT_FAILURE);
  }
  
  Maru_String infile = Maru_String_from_cstr(arena, argv[optind]);
  Maru_String text = Maru_read_all(arena, infile);
  Maru_Ast *ast = Maru_parse(arena, text);

  FILE *file = stdout;
  if (outfile.cstr != NULL) 
    file = fopen(outfile.cstr, "w");

  if (!do_dump_ast)
    Maru_emit(file, ast);
  else
  {
    dump_ast(file, ast);
    printf("\n");
  }

  if (file!=stdout)
    fclose(file);

  Maru_Arena_free(arena);
  return 0;
}
