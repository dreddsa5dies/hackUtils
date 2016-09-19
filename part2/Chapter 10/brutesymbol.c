#include <stdio.h>

int main()
{
  char pswd[10];
  int p = 0;
  pswd[0] = ' ';
  pswd[1] = 0;

  while(1)
  {        
    while ((++pswd[p]) > '~')
    { 
	pswd[p] = ' ';
	p++;
	if (!pswd[p])
	{
	  pswd[p] = ' ';
	  pswd[p + 1] = 0;
	}
    }
    p = 0;
    printf("%s\n", &pswd[0]);    
  }

  return 0;
}
