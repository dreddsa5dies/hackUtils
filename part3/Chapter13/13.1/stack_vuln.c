#include <stdio.h>
#include <string.h>

int main (int argc, char *argv[])
{
  char buf[100];

  if (argc > 1) {
    strcpy(buf, argv[1]);
    printf("OK!\n");
  } else
    printf("Please, enter the argument!\n");

  return 0;
}
