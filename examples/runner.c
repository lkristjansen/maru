#include <stdio.h>
#include <stdint.h>

int64_t maru_entry();

int main()
{
  int64_t val = maru_entry();

  if ((val & 0x3) == 0)
    printf("%ld", val >> 2);
  else if ((val & 0xff) == 0xf)
    printf("'%c'", (char)(val >> 8));
  else if ((val & 0x7f) == 0x1f)
    printf("#%c", (val >> 7) ? 't' : 'f');
  else if ((val & 0x3f) == 0x2f)
    printf("()");
  else
   printf("unknown type");

  return 0;
}