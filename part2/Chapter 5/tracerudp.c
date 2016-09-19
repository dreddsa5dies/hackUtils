#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <sys/time.h>
#include <unistd.h>

#define BUFSIZE 1500

/* структура данных UDP */
struct outdata {             
  int outdata_seq;           /* порядковый номер */
  int outdata_ttl;           /* значение TTL, с которым пакет отправляется */
  struct timeval outdata_tv; /* время отправки пакета */
};

char recvbuf[BUFSIZE];
char sendbuf[BUFSIZE];

int sendfd; /* дескриптор сокета для отправки UDP-дейтаграмм */
int recvfd; /* дескриптор сырого сокета для приема ICMP-сообщений */
struct sockaddr_in sasend;
 /* структура sockaddr() для отправки пакета */
struct sockaddr_in sabind;
 /* структура sockaddr() для связывания порта отправителя */
struct sockaddr_in sarecv;
 /* структура sockaddr() для получения пакета */
struct sockaddr_in salast;
 /* последняя структура sockaddr() для получения */
int sport;
int dport;

int ttl;
int probe;
int max_ttl = 30;
 /* максимальное значение поля TTL */
int nprobes = 3;
  /* количество пробных пакетов */
int dport = 32768 + 666;
 /* начальный порт получателя */
int datalen = sizeof(struct outdata);
 /* длина поля данных UDP */

/* прототипы функций */
void tv_sub(struct timeval *, struct timeval *);

int packet_ok(int, struct timeval *)
;

/*------------------------*/
/* Главная функция main() */
/*------------------------*/
int main(int argc, char *argv[])
{
  int seq;
  int code;
  int done;
  double rtt;
  struct hostent *hp;
  struct outdata *outdata;
  struct timeval tvrecv;
  
  if (argc != 2)
 {
    fprintf(stderr, "Usage: %s <hostname>\n", argv[0]);
    exit(-1);
  }

  if ( (hp = gethostbyname(argv[1])) == NULL) {
    herror("gethostbyname() failed");
    exit(-1);
  }

  if ( (recvfd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
    perror("socket() failed");
    exit(-1);
  }

  /* восстановление исходных прав */
  setuid(getuid());

  if ( (sendfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket() failed");
    exit(-1);
  }

  sport = (getpid() & 0xffff) | 0x8000;
 /* номер порта UDP отправителя */

  bzero(&sasend, sizeof(sasend));
  sasend.sin_family = AF_INET;
  sasend.sin_addr= *((struct in_addr *) hp->h_addr);

  sabind.sin_family = AF_INET;
  sabind.sin_port = htons(sport);
  if (bind(sendfd, (struct sockaddr *)&sabind, sizeof(sabind)) != 0)
    perror("bind() failed");



  seq = 0;
  done = 0;
  for (ttl = 1; ttl <= max_ttl && done == 0; ttl++) {
    setsockopt(sendfd, SOL_IP, IP_TTL, &ttl, sizeof(int));
    bzero(&salast, sizeof(salast));

    printf("%2d  ", ttl);
    fflush(stdout);

    for (probe = 0; probe < nprobes; probe++) {
      outdata = (struct outdata *)sendbuf;
      outdata->outdata_seq = ++seq;
      outdata->outdata_ttl = ttl;
      gettimeofday(&outdata->outdata_tv, NULL);
    
      sasend.sin_port = htons(dport + seq);

      if (sendto(sendfd, sendbuf, datalen, 0, (struct sockaddr *)&sasend, sizeof(sasend)) < 0) {
        perror("sendto() failed");
        exit(-1);
      }

      if ( (code = packet_ok(seq, &tvrecv)) == -3)
 
        printf(" *"); /* истечение времени ожидания, нет ответа */
      else {
        if ( memcmp(&sarecv.sin_addr, &salast.sin_addr, sizeof(sarecv.sin_addr)) != 0) {
 
          if ( (hp = gethostbyaddr(&sarecv.sin_addr, sizeof(sarecv.sin_addr), sarecv.sin_family)) != 0)
            printf(" %s (%s)", inet_ntoa(sarecv.sin_addr), hp->h_name);
          else
            printf(" %s", inet_ntoa(sarecv.sin_addr));
          memcpy(&salast.sin_addr, &sarecv.sin_addr, sizeof(salast.sin_addr));
        }

        tv_sub(&tvrecv, &outdata->outdata_tv);
        rtt = tvrecv.tv_sec * 1000.0 + tvrecv.tv_usec / 1000.0;
        printf("  %.3f ms", rtt);

        if (code == -1)
          ++done;
      }
      
      fflush(stdout);
    }
 
    printf("\n");
  }

  return 0;
}

/*
---------------------------------------------------------------*/
/* Разбор принятого пакета                                       */
/*                                                               */
/* Возвращает:                                                   */
/* -3 в случае истечения времени ожидания;                       */
/* -2 в случае получения сообщения ICMP time exceeded in transit */
/*    (программа продолжает работать);                           */
/* -1 в случае получения сообщения ICMP port unreachable         */
/*    (программа завершается)
.                                   */
/*---------------------------------------------------------------*/

int packet_ok(int seq, struct timeval *tv)
{
  int n;
  int len;
  int hlen1;
  int hlen2;
  struct ip *ip;
  struct ip *hip;
  struct icmp *icmp;
  struct udphdr *udp;
  fd_set fds;
  struct timeval wait;

  wait.tv_sec = 4;
 /* ждать ответа не более 4-х секунд */
  wait.tv_usec = 0;

  for (;;) {
    len = sizeof(sarecv);

    FD_ZERO(&fds);
    FD_SET(recvfd, &fds);

    if (select(recvfd+1, &fds, NULL, NULL, &wait) > 0)
      n = recvfrom(recvfd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*)&sarecv, &len);
    else if (!FD_ISSET(recvfd, &fds))
      return (-3);
    else
      perror("recvfrom() failed");

    gettimeofday(tv, NULL);

    ip = (struct ip *) recvbuf;	/* начало IP-заголовка */
    hlen1 = ip->ip_hl << 2;     /* длина IP-заголовка */
	
    icmp = (struct icmp *) (recvbuf + hlen1); /* начало ICMP-заголовка */
    /* начало сохраненного IP-заголовка */
    hip = (struct ip *) (recvbuf + hlen1 + 8);
    /* длина сохраненного IP-заголовка */
    hlen2 = hip->ip_hl << 2;
    /* начало сохраненного UDP-заголовка */
    udp = (struct udphdr *) (recvbuf + hlen1 + 8 + hlen2);
	
    if (icmp->icmp_type == ICMP_TIMXCEED &&
	icmp->icmp_code == ICMP_TIMXCEED_INTRANS) {
      if (hip->ip_p == IPPROTO_UDP &&
          udp->source == htons(sport) &&
          udp->dest == htons(dport + seq))
        return (-2);
    } 
    
    if (icmp->icmp_type == ICMP_UNREACH) {
      if (hip->ip_p == IPPROTO_UDP &&
        udp->source == htons(sport) &&
        udp->dest == htons(dport + seq)) {		
	if (icmp->icmp_code == ICMP_UNREACH_PORT)
          return (-1);
      }      
    }
  }	
}



/*---------------------------------*/
/* Вычитание двух timeval структур */
/*---------------------------------*/
void tv_sub(struct timeval *out, struct timeval *in)
{
  if ( (out->tv_usec -= in->tv_usec) < 0) {
    out->tv_sec--;
    out->tv_usec += 1000000;
  }  
  out->tv_sec -= in->tv_sec;
}
