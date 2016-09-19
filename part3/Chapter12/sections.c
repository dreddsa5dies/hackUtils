#include <stdio.h>
#include <stdlib.h>

int var;
char *str;
int x = 111;
static int y = 222;
char buffer1[666];
char mes1[] = "abcdef";

void f(int a, char *b)
{
  char p;
  int num = 333;
  static int count = 444;
  char buffer2[777];
  char mes2[] = "zyxwvu";

  str = malloc(1000*sizeof(char));
  strncpy(str, "abcde", 5);
  strncpy(buffer1, "Sklyaroff", 9);
  strncpy(buffer2, "Ivan", 4);
}

int main()
{
  f(1, "string");
  
  return 0;
}
