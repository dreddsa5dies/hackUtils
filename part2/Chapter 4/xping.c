#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>

#define BUFSIZE 1500

int sd;
                      /* дескриптор сокета */
pid_t pid;                   /* идентификатор нашего процесса PID */
struct sockaddr_in servaddr; /* структура sockaddr() для отправки пакета */
struct sockaddr_in from;     /* структура sockaddr() для получения пакета */

double tmin = 999999999.0;   /* минимальное время обращения */
double tmax = 0;             /* максимальное время обращения */
double tsum = 0;             /* сумма всех времен для вычисления среднего времени */
int nsent = 0;               /* количество отправленных пакетов */
int nreceived = 0;           /* количество полученных пакетов */

/* прототипы функций */
void pinger(void)
;
void output(char *, int, struct timeval *)
;
void catcher(int);
void tv_sub(struct timeval *, struct timeval *)
;
unsigned short in_cksum(unsigned short *, int)
;

/*------------------------*/
/* Главная функция main() */
/*------------------------*/
int
 main(int argc, char *argv[])
{

  int size;
  int fromlen;
  int n;
  struct timeval tval;
  char recvbuf[BUFSIZE];
  struct hostent *hp;
  struct sigaction act;
  struct itimerval timer;
  const int on = 1;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <hostname>\n", argv[0]);
    exit(-1);
  }

  pid = getpid();

  /* установка обработчика сигналов SIGALRM и SIGINT */	
  memset(&act, 0, sizeof(act));
  /* обработчиком назначается функция catcher() */
  act.sa_handler = &catcher;
  sigaction(SIGALRM, &act, NULL);
  sigaction(SIGINT, &act, NULL);
	
  if ( (hp = gethostbyname(argv[1])) == NULL) {
    herror("gethostbyname() failed");
    exit(-1);
  }

  if ( (
sd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
    perror("socket() failed");
    exit(-1);
  }

  /* восстановление исходных прав */
  setuid(getuid());

  /* даем возможность посылать широковещательные сообщения */
  setsockopt(sd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));

  
  /* увеличиваем размер приемного буфера */
  size = 60 *1024;
  setsockopt(sd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));


  /* запускаем интервальный таймер, посылающий сигнал SIGALRM */
  /* таймер сработает через 1 микросекунду... */
  timer.it_value.tv_sec=0;
  timer.it_value.tv_usec=1;
  /* ... и будет активироваться каждую секунду */
  timer.it_interval.tv_sec=1;
  timer.it_interval.tv_usec=0;
  /* запуск таймера реального времени */
  setitimer(ITIMER_REAL, &timer, NULL);
	
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr= *((struct in_addr *) hp->h_addr);

  fromlen = sizeof(from);
 
  /* запускаем бесконечный цикл, в котором будем принимать пакеты */
  while (1) {
    n = recvfrom(sd, recvbuf, sizeof(recvbuf), 0, 
		(struct sockaddr *)&from, &fromlen);
		
    if (n < 0) {
      if (errno == EINTR)
        continue;
      perror("recvfrom() failed");
      continue;
    }

    
    /* определяем текущее системное время */
    gettimeofday(&tval, NULL);
    
    /* вызываем функцию для разбора принятого пакета и вывода данных на экран */
    output(recvbuf, n, &tval);
  }

  return 0;
}

/*---------------------------------------*/
/* Разбор пакета и вывод данных на экран */
/*---------------------------------------*/
void output(char *ptr, int len, struct timeval *tvrecv)
{
  int iplen;
  int icmplen;
  struct ip *ip;
  struct icmp *icmp;
  struct timeval *tvsend;
  double rtt;

  ip = (struct ip *) ptr; /* начало IP-заголовка */
  iplen = ip->ip_hl << 2; /* длина IP-заголовка */

  icmp = (struct icmp *) (ptr + iplen); /* начало ICMP-заголовка */
  if ( (icmplen = len - iplen) < 8)     /* длина ICMP-заголовка */
    fprintf(stderr, "icmplen (%d) < 8", icmplen);

  if (icmp->icmp_type == ICMP_ECHOREPLY) {
  
    if (icmp->icmp_id != pid)
      return; /* ответ не на наш запрос ECHO REQUEST */

    tvsend = (struct timeval *) icmp->icmp_data;
    tv_sub(tvrecv, tvsend);
    
    /* время оборота пакета (round-trip time) */
    rtt = tvrecv->tv_sec * 1000.0 + tvrecv->tv_usec / 1000.0;
    
    nreceived++;
				
    tsum += rtt;
    if (rtt < tmin)
      tmin = rtt;
    if (rtt > tmax)
      tmax = rtt;

    printf("%d bytes from %s: icmp_seq=%u, ttl=%d, time=%.3f ms\n",
	    icmplen, inet_ntoa(from.sin_addr),
 
	    icmp->icmp_seq, ip->ip_ttl, rtt);
  }

}


/*-------------------------------------------------*/
/* Формирование и отсылка ICMP ECHO REQUEST пакета */
/*-------------------------------------------------*/
void pinger(void)
{
  int icmplen;
  struct icmp *icmp;
  char sendbuf[BUFSIZE];

  icmp = (struct icmp *) sendbuf;
  
  /* заполняем все поля ICMP-сообщения */
  icmp->icmp_type = ICMP_ECHO;
  icmp->icmp_code = 0;
  icmp->icmp_id = pid;
  icmp->icmp_seq = nsent++;
  gettimeofday((struct timeval *) icmp->icmp_data, NULL);
  
  /* длина ICMP-заголовка составляет 8 байт и 56 байт данных */
  icmplen = 8 + 56;
  /* контрольная сумма ICMP-заголовка и данных */
  icmp->icmp_cksum = 0;
  icmp->icmp_cksum = in_cksum((unsigned short *) icmp, icmplen);

  if (sendto(sd, sendbuf, icmplen, 0, 
    (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("sendto() failed");
    exit(-1);
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

/*--------------------------------------*/
/* Обработчик сигналов SIGALRM и SIGINT */
/*--------------------------------------*/

void catcher(int signum)
{
  if (signum == SIGALRM)
  {
    pinger();
 
    return;
            
  } else if (signum == SIGINT) {
    printf("\n--- %s ping statistics ---\n", inet_ntoa(servaddr.sin_addr));
    printf("%d packets transmitted, ", nsent);
    printf("%d packets received, ", nreceived);
    if (nsent)
 {
      if (nreceived > nsent)
	printf("-- somebody's printing up packets!");
      else
	printf("%d%% packet loss", 
	                  (int) (((nsent-nreceived)*100) /
			  nsent));
    }
    printf("\n");
    if (nreceived)
      printf("round-trip min/avg/max = %.3f/%.3f/%.3f ms\n",
		tmin,
		tsum / nreceived,
		tmax);
    fflush(stdout);
    exit(-1);
  }
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
    
  sum = (sum >> 16) + (sum & 0xFFFF); /* добавляем перенос */
  sum += (sum >> 16);                 /* еще раз */
  result = ~sum;                      /* инвертируем результат */
  return result;
}

