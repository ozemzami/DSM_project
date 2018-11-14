#ifndef COMMON_IMPL_H_
#define COMMON_IMPL_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>

/* autres includes (eventuellement) */

#define ERROR_EXIT(str) {perror(str);exit(EXIT_FAILURE);}

/* definition du type des infos */
/* de connexion des processus dsm */
struct dsm_proc_conn  {
   int rank;
   int fd;
   int port;
   char hostname[20];
};
typedef struct dsm_proc_conn dsm_proc_conn_t;

/* definition du type des infos */
/* d'identification des processus dsm */
struct dsm_proc {
  pid_t pid;
  dsm_proc_conn_t connect_info;
};
typedef struct dsm_proc dsm_proc_t;

int creer_socket(int type, int *port_num);
int do_socket(int domain, int type, int protocol);
void init_serv_addr(int port, struct sockaddr_in *serv_addr);
int do_bind(struct sockaddr_in serv_adr , int sockfd);
int do_listen( int socket , int backlog);
int do_accept(int sockfd , struct sockaddr* serv_adr , socklen_t* addrlen);
int do_read(int sockfd, char* buffer, int len);
int do_write (int fd,const void *buffer, size_t count);
struct sockaddr_in get_addr_info(char *hostname, int portnumber);
void do_connect(int sock, struct sockaddr_in address);
void error(const char *msg);

#endif
