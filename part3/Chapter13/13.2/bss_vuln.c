#include <stdio.h>
#include <string.h>

void show(char *);

int main(int argc, char *argv[])
{
  static char buf[100];
  static void (*func_ptr)(char *arg);

  if (argc < 2) {
    printf("Usage: %s <buffer data>\n", argv[0]);
    exit(1);
  }

  func_ptr = show;
  strncpy(buf, argv[1], strlen(argv[1]));
  func_ptr(buf);

  return 0;
}

void show(char *arg)
{
  printf("\nBuffer: [%s]\n\n", arg);
}
