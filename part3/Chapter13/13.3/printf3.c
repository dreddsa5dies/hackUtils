#include <stdio.h>

int main()
{
  int n;

  printf("ABCDEFGHIJ%n\n", &n);
  printf("n=%d\n", n);

  return 0;
}
