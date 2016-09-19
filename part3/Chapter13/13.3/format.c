#include <stdio.h>

int main(int argc, char *argv[])
{
  int a = 1;
  char buf[100];
  int b = 1;

  printf ("a=%d (%p)\n", a, &a);
  printf ("b=%d (%p)\n", b, &b);

  snprintf(buf, sizeof buf, argv[1]);

  printf("\nbuf: [%s]\n\n", buf);
  printf ("a=%d (%p)\n", a, &a);
  printf ("b=%d (%p)\n", b, &b);

  return 0;
}
