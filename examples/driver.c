#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PIPE_READ 0
#define PIPE_WRITE 1

static int failures = 0;

void run_example(const char *infile, const char *expected)
{
  printf("[TEST] %s ... ", infile);
  fflush(stdout);

  pid_t pid = fork();
  if (pid == 0)
  {
    execl("bin/maru", "bin/maru", "-o", "bin/example.s", infile, NULL);
    exit(EXIT_FAILURE);
  }
  else
  {
    wait(NULL);
  }

  pid = fork();
  if (pid == 0)
  {
    execl("/usr/bin/cc", "/usr/bin/cc", "-o", "bin/example", "bin/example.s", "examples/runner.c", NULL);
    exit(EXIT_FAILURE);
  }
  else
  {
    wait(NULL);
  }

  int fd[2] = {0};
  pipe(fd);

  pid = fork();
  if (pid == 0)
  {
    close(fd[PIPE_READ]);
    dup2(fd[PIPE_WRITE], STDOUT_FILENO);
    execl("bin/example", "bin/example", NULL);
    exit(EXIT_FAILURE);
  }
  else
  {
    close(fd[PIPE_WRITE]);
    wait(NULL);

    char buf[1024] = {0};
    read(fd[PIPE_READ], buf, sizeof(buf) - 1);

    close(fd[PIPE_READ]);

    if (strncmp(expected, buf, strlen(expected)) == 0)
      printf("PASSED\n");
    else
    {
      failures += 1;
      printf("FAILED\n");
      printf("\texpected: '%s'\n", expected);
      printf("\tgot: '%s'\n", buf);
    }
  }
}

int main()
{
  printf("\n== RUNNING examples/immediate ==\n");
  run_example("examples/immediate/fixnum.l", "500");
  run_example("examples/immediate/fixnum_neg.l", "-400");
  run_example("examples/immediate/nil.l", "()");
  run_example("examples/immediate/true.l", "#t");
  run_example("examples/immediate/false.l", "#f");
  run_example("examples/immediate/char.l", "'d'");

  printf("\n== RUNNING examples/unary_primitive ==\n");
  run_example("examples/unary_primitives/is_zero_fixnum_true.l", "#t");
  run_example("examples/unary_primitives/is_zero_fixnum_false.l", "#f");
  run_example("examples/unary_primitives/is_null_true.l", "#t");
  run_example("examples/unary_primitives/is_null_fixnum.l", "#f");
  run_example("examples/unary_primitives/is_null_bool.l", "#f");
  run_example("examples/unary_primitives/is_null_char.l", "#f");
  run_example("examples/unary_primitives/add1.l", "301");
  run_example("examples/unary_primitives/sub1.l", "299");
  run_example("examples/unary_primitives/char_to_integer.l", "65");
  run_example("examples/unary_primitives/integer_to_char.l", "'A'");
  run_example("examples/unary_primitives/is_integer_true.l", "#t");
  run_example("examples/unary_primitives/is_integer_nil.l", "#f");
  run_example("examples/unary_primitives/is_integer_char.l", "#f");
  run_example("examples/unary_primitives/is_integer_bool.l", "#f");
  run_example("examples/unary_primitives/is_bool_true.l", "#t");
  run_example("examples/unary_primitives/is_bool_false.l", "#t");
  run_example("examples/unary_primitives/is_bool_fixnum.l", "#f");
  run_example("examples/unary_primitives/is_bool_char.l", "#f");
  run_example("examples/unary_primitives/is_bool_nil.l", "#f");
  run_example("examples/unary_primitives/is_char_true.l", "#t");
  run_example("examples/unary_primitives/is_char_fixnum.l", "#f");
  run_example("examples/unary_primitives/is_char_bool.l", "#f");
  run_example("examples/unary_primitives/is_char_nil.l", "#f");

  return failures;
}
