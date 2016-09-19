#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BUFFER_SIZE 1000
#define NAME_SIZE 2000

hello_client(int sock)
{
  char buf[BUFFER_SIZE];
  char name[NAME_SIZE];
  int nbytes;

  strcpy(buf, "Your name: ");
  send(sock, buf, strlen(buf), 0);

  if ( (nbytes = recv(sock, name, sizeof(name), 0)) > 0) {
    name[nbytes-1] = '\0';
    sprintf(buf, "Hello %s\r\n", name);
    send(sock, buf, strlen(buf), 0);
  }
}

int main(int argc, char *argv[])
{
  int sd;
  int clisd;
  struct sockaddr_in servaddr;

  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    exit(-1);
  }

  if ( (sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket() failed");
    exit(-1);
  }
  
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(atoi(argv[1]));

  if (bind(sd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
    perror("bind() failed");
    exit(-1);
  }

  if (listen(sd, 30) != 0) {
    perror("listen() failed");
    exit(-1);
  }

  for(;;) {
    if ( (clisd = accept(sd, NULL, NULL)) < 0) {
      perror("accept() failed");
      exit(-1);
    }
    
    hello_client(clisd);  
    close(clisd);
  }
  
  return 0;
}
