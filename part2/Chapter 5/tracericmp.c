#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>

#define BUFSIZE 1500

int sd;   
 /* дескриптор сокета */
pid_t pid;
 /* идентификатор нашего процесса PID */
struct sockaddr_in sasend;
 /* структура sockaddr() для отправки пакета */
struct sockaddr_in sarecv;
 /* структура sockaddr() для получения пакета */
struct sockaddr_in salast; 
/* последняя структура sockaddr() для получения */

int ttl;
int probe;
int max_ttl = 30;
 /* максимальное значение поля TTL */
int nprobes = 3; 
 /* количество пробных пакетов */

/* прототипы функций */
int output(int, struct timeval *);
void tv_sub(struct timeval *, struct timeval *);
unsigned short in_cksum(unsigned short *, int);

/*------------------------*/
/* Главная функция main() */
/*------------------------*/
int main(int argc, char *argv[])
{
  int seq;
  int code;
  int done;
  double rtt;
  struct timeval *tvsend;
  struct timeval tvrecv;
  struct hostent *hp;  
  int icmplen;
  struct icmp *icmp;
  char sendbuf[BUFSIZE];

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <hostname>\n", argv[0]);
    exit(-1);
  }

  pid = getpid();

  if ( (hp = gethostbyname(argv[1])) == NULL) {
    herror("gethostbyname() failed");
    exit(-1);
  }

  if ( (sd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
    perror ("socket() failed");
    exit(-1);
  }
	
  setuid(getuid());

  bzero(&sasend, sizeof(sasend));
  sasend.sin_family = AF_INET;
  sasend.sin_addr= *((struct in_addr *) hp->h_addr);


  seq = 0;
  done = 0;
  for (ttl = 1; ttl <= max_ttl && done == 0; ttl++) {
    setsockopt(sd, SOL_IP, IP_TTL, &ttl, sizeof(int));
    bzero(&salast, sizeof(salast));

    printf("%2d  ", ttl);
    fflush(stdout);

    for (probe = 0; probe < nprobes; probe++) {

      icmp = (struct icmp *) sendbuf;
      
      /* заполняем все поля ICMP-сообщения */
      icmp->icmp_type = ICMP_ECHO;
      icmp->icmp_code = 0;
      icmp->icmp_id = pid;
      icmp->icmp_seq = ++seq;
      tvsend = (struct timeval *) icmp->icmp_data;
      gettimeofday(tvsend, NULL);
      
      /* длина ICMP-заголовка составляет 8 байт и 56 байт данных */
      icmplen = 8 + 56;
      /* контрольная сумма ICMP-заголовка и данных */
      icmp->icmp_cksum = 0;
      icmp->icmp_cksum = in_cksum((unsigned short *) icmp, icmplen);

      if (sendto(sd, sendbuf, icmplen, 0, (struct sockaddr *)&sasend, sizeof(sasend)) < 0) {
        perror("sendto() failed");
        exit(-1);
      }
      
      if ( (code = output(seq, &tvrecv)) == -3)
        printf(" *");
      else {
        if (memcmp(&sarecv.sin_addr, &salast.sin_addr, sizeof(sarecv.sin_addr)) != 0) {
          if ( (hp = gethostbyaddr(&sarecv.sin_addr, sizeof(sarecv.sin_addr), sarecv.sin_family)) != 0)
            printf(" %s (%s)", inet_ntoa(sarecv.sin_addr), hp->h_name);
          else
            printf(" %s", inet_ntoa(sarecv.sin_addr));
          memcpy(&salast.sin_addr, &sarecv.sin_addr, sizeof(salast.sin_addr));
        }
	
        tv_sub(&tvrecv, tvsend);
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

/*---------------------------------------------------------------*/
/* Разбор принятого пакета                                       */
/*                                                               */
/* Возвращает:                                                   */
/* -3 в случае истечения времени ожидания;                       */
/* -2 в случае получения сообщения ICMP time exceeded in transit */
/*    (программа продолжает работать);                           */
/* -1 в случае получения сообщения ICMP Echo Reply               */
/*    (программа завершается).                                   */
/*---------------------------------------------------------------*/
int output(int seq, struct timeval *tv)
{

  int n;
  int len;
  int hlen1;
  int hlen2;
  struct ip *ip;
  struct ip *hip;
  struct icmp *icmp;
  struct icmp *hicmp;
  double rtt;
  char recvbuf[BUFSIZE];
  fd_set fds;
  struct timeval wait;

  wait.tv_sec = 4;
 /* ждать ответа не более 4-х секунд */
  wait.tv_usec = 0;

  for (;;) {
    len = sizeof(sarecv);

    FD_ZERO(&fds);
    FD_SET(sd, &fds);

    if (select(sd+1, &fds, NULL, NULL, &wait) > 0)
      n = recvfrom(sd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*)&sarecv, &len);

    else if (!FD_ISSET(sd, &fds))
      return (-3);
    else
      perror("recvfrom() failed");

    gettimeofday(tv, NULL);

    ip = (struct ip *) recvbuf;	/* начало IP-заголовка */
    hlen1 = ip->ip_hl << 2;     /* длина IP-заголовка */
	
    icmp = (struct icmp *) (recvbuf + hlen1); /* начало ICMP-заголовка */

    
if (icmp->icmp_type == ICMP_TIMXCEED && 
        icmp->icmp_code == ICMP_TIMXCEED_INTRANS) {
      hip = (struct ip *)(recvbuf + hlen1 + 8);
      hlen2 = hip->ip_hl << 2;
      hicmp = (struct icmp *) (recvbuf + hlen1 + 8 + hlen2);
      if (hicmp->icmp_id == pid && hicmp->icmp_seq == seq)
 
        return (-2);
    }

    if (icmp->icmp_type == ICMP_ECHOREPLY &&
        icmp->icmp_id == pid &&
        icmp->icmp_seq == seq)
      return (-1);
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

/*------------------------------*/
/* Вычисление контрольной суммы */

/*------------------------------*/
unsigned short in_cksum(unsigned short *addr, int len)
{
  unsigned short result;
  unsigned int sum = 0;

  /* складываем все двухбайтовые слова */    
  while (len > 1) {
    sum += *addr++;
    len -= 2;
  }
  
  /* если остался лишний байт, прибавляем его к сумме */
  if (len == 1)
    sum += *(unsigned char*) addr;
    
  sum = (sum >> 16) + (sum & 0xFFFF);
 /* добавялем перенос */
  sum += (sum >> 16);
                 /* еще раз */
  result = ~sum;
                      /* инвертируем результат */
  return result;
}

