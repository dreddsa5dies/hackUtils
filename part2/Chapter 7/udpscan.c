#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <unistd.h>
#include <strings.h>

send_packet(int sendsock, unsigned short port, struct hostent* hp)
{
  struct sockaddr_in servaddr;
  char sendbuf[] = "Regard from Sklyaroff Ivan!";

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  servaddr.sin_addr = *((struct in_addr *)hp->h_addr);

  if (sendto(sendsock, sendbuf, sizeof(sendbuf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("sendto() failed");
  }
}

recv_packet(int recvsock)
{
  unsigned char recvbuf[1500];
  struct icmp *icmp;
  struct ip *iphdr;
  int iplen;
  fd_set fds;
  struct timeval wait;

  wait.tv_sec = 1;
  wait.tv_usec = 0;

  while(1)
  {
    FD_ZERO(&fds);
    FD_SET(recvsock, &fds);

    if (select(recvsock+1, &fds, NULL, NULL, &wait) > 0) {
      recvfrom(recvsock, &recvbuf, sizeof(recvbuf), 0x0, NULL, NULL);
    } else if (!FD_ISSET(recvsock, &fds))
      return 1;
    else
      perror("recvfrom() failed");

    iphdr = (struct ip *)recvbuf;
    iplen = iphdr->ip_hl << 2;		  
    icmp = (struct icmp *)(recvbuf + iplen);

    if ( (icmp->icmp_type == ICMP_UNREACH) && 
	 (icmp->icmp_code == ICMP_UNREACH_PORT))
      return 0;
  }
}

int main(int argc, char *argv[])
{
  int sendsock, recvsock;
  int port, portlow, porthigh;
  struct hostent* hp;
  unsigned int dest;
  struct servent* srvport;

  if (argc != 4) {
    fprintf(stderr, "Usage: %s <address> <portlow> <porthigh>\n", argv[0]);
    exit(-1);
  }

  hp = gethostbyname(argv[1]); 
  if (hp == NULL) {
    herror("gethostbyname() failed");
    exit(-1);
  }

  portlow = atoi(argv[2]);
  porthigh = atoi(argv[3]);

  if ( (sendsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    perror("sendsock failed");
    exit(-1);
  }

  if ( (recvsock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
    perror("recvsock failed");
    exit(-1);
  }
  
  fprintf(stderr, "Running scan...\n");

  for (port = portlow; port <= porthigh; port++) {
    send_packet(sendsock, port, hp);
    if (recv_packet(recvsock) == 1) {
      srvport = getservbyport(htons(port), "udp");
      if (srvport == NULL)
        printf("Open: %d (unknown)\n", port);
      else
        printf("Open: %d (%s)\n", port, srvport->s_name);
	
      fflush(stdout); 
    }
  }

  return 0;
}

