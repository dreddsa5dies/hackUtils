#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

unsigned short in_cksum(unsigned short *addr, int len)
{
  unsigned short result;
  unsigned int sum = 0;

  while (len > 1) {
    sum += *addr++;
    len -= 2;
  }

  if (len == 1)
    sum += *(unsigned char*) addr;

  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  result = ~sum;
  return result;
}

int main(int argc, char *argv[])
{
  int sd;
  const int on = 1;
  int type, port;
  struct sockaddr_in servaddr;
  
  char sendbuf[sizeof(struct iphdr) + sizeof(struct icmp)];
  struct iphdr *ip_hdr = (struct iphdr *)sendbuf;
  struct icmp *icmp_hdr = (struct icmp *) (sendbuf + sizeof(struct iphdr));

  port = 31337;
  type = 0;

  if ((argc < 3) || (argc > 5)) {
    fprintf(stderr, 
      "Usage: %s <srcip> <dstip> [port] [type]\n"
      "port - default 31337\n"
      "type - default Echo Reply(0).\n",
      argv[0]);
    exit(-1);
  }

  if (argc > 3)
    port = atoi(argv[3]);

  if (argc == 5)
    type = atoi(argv[4]);

  printf("Port: %d, Type: %d.\n", port, type);        

  sd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW);
  if (setsockopt(sd, IPPROTO_IP, IP_HDRINCL, (char *)&on, sizeof(on)) < 0) {
    perror("setsockopt() failed");
    exit(-1);
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(argv[2]);
	
  ip_hdr->ihl     = 5;
  ip_hdr->version = 4;
  ip_hdr->tos     = 0;
  ip_hdr->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmp));
  ip_hdr->id      = htons(getuid());
  ip_hdr->ttl      = 255;
  ip_hdr->protocol = IPPROTO_ICMP;
  ip_hdr->saddr    = inet_addr(argv[1]);
  ip_hdr->daddr    = inet_addr(argv[2]);
  ip_hdr->check = 0;
  ip_hdr->check = in_cksum((unsigned short *)ip_hdr, sizeof(struct iphdr));

  icmp_hdr->icmp_type = type;
  icmp_hdr->icmp_code = 0;
  icmp_hdr->icmp_id = 0xABCD;
  icmp_hdr->icmp_seq = port;
  icmp_hdr->icmp_cksum = 0;
  icmp_hdr->icmp_cksum = in_cksum((unsigned short *)icmp_hdr, sizeof(struct icmp));

  if(sendto(sd, 
	    sendbuf,
	    sizeof(sendbuf),
	    0,
	    (struct sockaddr *)&servaddr,
	    sizeof(servaddr)) < 0) {
    perror("sendto() failed");
    exit(-1);
  }
  printf("Packet successfuly sending.\n");
  close(sd);
}
