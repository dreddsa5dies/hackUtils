#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
  char *a;
  char *b;

  a = malloc(200);
  b = malloc(64);

  printf("a = %p, b = %p, b-a = %d\n\n", a, b, b-a);

  strcpy(a, argv[1]);

  printf("a = %s (%d)\n", a, strlen(a));
  printf("b = %s (%d)\n", b, strlen(b));

  free(a);
  free(b);

  return 0;
}
