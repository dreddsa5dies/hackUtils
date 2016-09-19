#include <stdio.h>

int main()
{
  char *str = "sklyaroff";
  int num = 31337;

  printf("str=%s, adrr_str=%p, num=%d, addr_num=%#x\n", str, &str, num, &num);

  return 0;
}
