#include <stdio.h>

int main()
{
  int n, x = 1;

  printf("ABCDEFGHIJ%.100d%n\n", x, &n);
  printf("n=%d\n", n);

  return 0;
}
