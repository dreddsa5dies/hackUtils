#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

char *port_host;
char *name;

void token(char *arg)
{
  name = strtok(arg, ":");
  port_host = strtok(NULL, "");
  
  if (port_host == NULL)
    port_host = "80";
}

int main(int argc, char* argv[])
{
  FILE *fd;
  int sd;
  int bytes;
  char buf[250];
  char str1[270];
  char str2[100];
  struct hostent* host;
  struct sockaddr_in servaddr;
  
  if (argc < 2 || argc > 3) {
    printf("Usage: %s host[:port] [proxy][:port]\n\n", argv[0]);
    exit(-1);
  }

  fprintf(stderr, "=====================================\n");  
  fprintf(stderr, "=  Simple command line CGI-scanner  =\n");
  fprintf(stderr, "=      by Ivan Sklyaroff, 2006.     =\n");
  fprintf(stderr, "=====================================\n");

  if (argc == 3)
    token(argv[2]);
  else
    token(argv[1]); 

  if ( (host = gethostbyname(name)) == NULL) {
    herror("gethostbyname() failed");
    exit(-1);
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(atoi(port_host));
  servaddr.sin_addr = *((struct in_addr *)host->h_addr);

  if( (fd = fopen("cgi-bugs.dat","r")) == NULL) {
    perror("fopen() failed");
    exit(-1);
  }

  fprintf(stderr, " Start scanning \"%s\"...\n", argv[1]);
  fprintf(stderr, "======================================\n");

  while (fgets(buf,250,fd) != NULL) {

    buf[strcspn(buf, "\r\n\t")] = 0;
    if (strlen(buf) == 0) continue;

    if ( (sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
      perror("socket() failed");
      exit(-1);
    }
        
    if (connect(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
      perror("connect() failed");
      exit(-1);
    }

    printf("======================================\n");
    
    if (argc == 2)
      sprintf(str1, "GET %s HTTP/1.1\r\n", buf);
    else
      sprintf(str1, "GET http://%s%s HTTP/1.1\r\n", argv[1], buf);

    sprintf(str2, "Host:%s\r\n\r\n", argv[1]);
     
    send(sd, str1, strlen(str1), 0);
    printf("%s", str1);
    send(sd, str2, strlen(str2), 0);
    printf("%s", str2);

    bzero(buf, 250);
    
    bytes = recv(sd, buf, sizeof(buf)-1, 0);
    buf[bytes] = 0;
    printf("%s\n", buf);

    if (strstr(buf, "200 OK") != NULL)
      printf("\nResult: FOUND!!!\n\n");
    else
      printf("\nResult: Not Found.\n\n");

    printf("======================================\n");

    close(sd); 
  }

  fprintf(stderr, "======================================\n");
  fprintf(stderr, " End scan \"%s\".\n", argv[1]);
  fprintf(stderr, "======================================\n");

  fclose(fd);

  return 0;
}
