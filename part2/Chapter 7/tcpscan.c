#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

int main(int argc, char *argv[])
{
  int sd;
  struct hostent* hp;
  struct sockaddr_in servaddr;
  struct servent *srvport;
  int port, portlow, porthigh;

  if (argc != 4) {
    fprintf(stderr, "Usage: %s <address> <portlow> <porthigh>\n", argv[0]);
    exit(-1);
  }

  hp = gethostbyname(argv[1]); 
  if (hp == NULL) {
    herror("gethostbyname() failed");
    exit(-1);
  }

  portlow  = atoi(argv[2]);
  porthigh = atoi(argv[3]);
  
  fprintf(stderr, "Running scan...\n");

  for (port = portlow; port <= porthigh; port++)
  {
    if ( (sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
      perror("socket() failed");
      exit(-1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr = *((struct in_addr *)hp->h_addr);

    if (connect(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == 0) {
      srvport = getservbyport(htons(port), "tcp");
      if (srvport == NULL)
        printf("Open: %d (unknown)\n", port);
      else
        printf("Open: %d (%s)\n", port, srvport->s_name);
      fflush(stdout); 
    }
    close(sd);
  }
  printf("\n");

  return 0;
}
