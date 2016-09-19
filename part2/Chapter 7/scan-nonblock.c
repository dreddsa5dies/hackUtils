#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SOCK 50

open_port(int port)
{
  struct servent *srvport;
  srvport = getservbyport(htons(port), "tcp");
  if (srvport == NULL)
    printf("Open: %d (unknown)\n", port);
  else
    printf("Open: %d (%s)\n", port, srvport->s_name);
  fflush(stdout);
}

main (int argc, char *argv[])
{
  struct usock_descr{
    int sd;                    // socket
    int state;                 // current socket state
    long timestamp;            // time, when sockets open in ms.
    unsigned short remoteport; // remote port
  };

  struct usock_descr sockets[MAX_SOCK];
  struct hostent* hp;
  struct sockaddr_in servaddr;
  struct timeval tv = {0,0};
  fd_set rfds, wfds;
  int i, flags, max_fd;
  int port, PORT_LOW, PORT_HIGH;
  int MAXTIME;

  if (argc != 5) {
    fprintf(stderr, "Usage: %s <ip> <portlow> <porthigh> <timeout in sec>\n", 
	    argv[0]);
    exit(-1);
  }
  
  hp = gethostbyname(argv[1]);
  if (hp == NULL) {
    herror("gethostbyname() failed");
    exit(-1);
  }

  PORT_LOW = atoi(argv[2]);
  PORT_HIGH = atoi(argv[3]) + 1;
  MAXTIME = atoi(argv[4]);

  fprintf(stderr, "Running scan...\n");

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr = *((struct in_addr *)hp->h_addr);

  port = PORT_LOW;
  for (i = 0; i < MAX_SOCK; i++)
    sockets[i].state = 0;

  while (port < PORT_HIGH) {
    for (i = 0; (i < MAX_SOCK) && (port < PORT_HIGH); i++) {
      if (sockets[i].state == 0) {
        if ( (sockets[i].sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
	  perror("socket() failed");
	  exit(-1);
	}
        flags = fcntl(sockets[i].sd, F_GETFL, 0);
        if(fcntl(sockets[i].sd, F_SETFL, flags | O_NONBLOCK) == -1) {
	  perror("fcntl() -- could not set nonblocking");
	  exit(-1);
	}
        sockets[i].state = 1;
      }
    } 
          
    for (i = 0; (i < MAX_SOCK) && (port < PORT_HIGH); i++) {
      if (sockets[i].state == 1) {
        servaddr.sin_port = ntohs(port);
        if (connect(sockets[i].sd, (struct sockaddr *)&servaddr, sizeof (servaddr)) == -1) {
          if (errno != EINPROGRESS) {
            shutdown(sockets[i].sd, 2);
            close(sockets[i].sd);
            sockets[i].state = 0;
          } else 
	    sockets[i].state = 2;
        } else {
	  open_port(port);
          shutdown(sockets[i].sd, 2);
          close(sockets[i].sd);
          sockets[i].state = 0;
        }
        sockets[i].timestamp = time(NULL);
        sockets[i].remoteport = port;
        port++;
      }
    }

    FD_ZERO(&rfds); 
    FD_ZERO(&wfds);
    max_fd = -1;

    for (i = 0; i < MAX_SOCK; i++) {
      if (sockets[i].state == 2) {
        FD_SET(sockets[i].sd, &wfds);
        FD_SET(sockets[i].sd, &rfds);
        if (sockets[i].sd > max_fd)
	  max_fd = sockets[i].sd;
      }
    }

    select(max_fd + 1, &rfds, &wfds, NULL, &tv);

    for (i = 0; i < MAX_SOCK; i++) {
      if (sockets[i].state == 2) {       
        if (FD_ISSET(sockets[i].sd, &wfds) || FD_ISSET(sockets[i].sd, &rfds)) {
          int error;
          socklen_t err_len = sizeof(error);
          if (getsockopt(sockets[i].sd, SOL_SOCKET, SO_ERROR, &error, &err_len) < 0 || error != 0) {
            shutdown(sockets[i].sd, 2);
            close(sockets[i].sd);
            sockets[i].state = 0;
          } else {
	    open_port(sockets[i].remoteport);
            shutdown(sockets[i].sd, 2);
            close(sockets[i].sd);
            sockets[i].state = 0;
          }
        } else {
	  if ( (time(NULL) - sockets[i].timestamp) > MAXTIME) {  
            shutdown(sockets[i].sd, 2);
            close(sockets[i].sd);
            sockets[i].state = 0;   
          }
        }
      }
    }
  }
    
  return 0;
}

