#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>

#define THREADS_MAX 255

int port, portlow, porthigh;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *scan(void *arg)
{
  int sd;
  struct sockaddr_in servaddr;
  struct servent *srvport;
  struct hostent* hp;

  char *argv1 = (char*)arg;
  
  hp = gethostbyname(argv1);
  if (hp == NULL) {
    herror("gethostbyname() failed");
    exit(-1);
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr = *((struct in_addr *)hp->h_addr);

  while (port < porthigh)
  {
    if ( (sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
      perror("socket() failed");
      exit(-1);
    }

    pthread_mutex_lock(&lock);
    servaddr.sin_port = htons(port);
            
    if (connect(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == 0) {
      srvport = getservbyport(htons(port), "tcp");
      if (srvport == NULL)
        printf("Open: %d (unknown)\n", port);
      else
        printf("Open: %d (%s)\n", port, srvport->s_name);
      fflush(stdout);
    }

    port++;
    close(sd);
    pthread_mutex_unlock(&lock);
  }
}

int main(int argc, char *argv[])
{
  pthread_t threads[THREADS_MAX];
  int thread_num;
  int i;
  
  if (argc != 5) {
    fprintf(stderr, "Usage: %s <address> <portlow> <porthigh> <num threads>\n", argv[0]);
    exit(-1);
  }

  thread_num = atoi(argv[4]);
  if (thread_num > THREADS_MAX)
    fprintf(stderr, "too many threads requested");

  portlow  = atoi(argv[2]);
  porthigh = atoi(argv[3]);
  port     = portlow;
  
  fprintf(stderr, "Running scan...\n");
  
  for (i = 0; i < thread_num; i++)
    if (pthread_create(&threads[i], NULL, scan, argv[1]) != 0)
      fprintf(stderr, "error creating thread");

  for (i = 0; i < thread_num; i++)
    pthread_join(threads[i], NULL);

  return 0;
}
