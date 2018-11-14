#include "common_impl.h"

int creer_socket(int type, int* port_num)
{
   int fd = 0;
   struct sockaddr_in serv_adr;
   socklen_t addrlen;

   /* fonction de creation et d'attachement */
   /* d'une nouvelle socket */
   fd=do_socket(AF_INET,type , 0);
   init_serv_addr( 0,&serv_adr);
   do_bind(serv_adr ,fd);

   /* renvoie le numero de descripteur */
   /* et modifie le parametre port_num */

   addrlen=sizeof(serv_adr);
   getsockname(fd,(struct sockaddr *) &serv_adr, &addrlen);
   *(port_num)=ntohs(serv_adr.sin_port);
   return fd;
}

/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */

/////creation of the socket////

int do_socket(int domain, int type, int protocol) {
  int sockfd;
  int yes = 1;
  sockfd = socket( domain, type , protocol );

  if ( sockfd ==-1) {
    ERROR_EXIT("Impossible to create the socket");
  }
  // set socket option, to prevent "already in use" issue when rebooting the server right on
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
  ERROR_EXIT("ERROR setting socket options");
  return sockfd;
}

///initialisation of the  variables/////
void init_serv_addr(int port, struct sockaddr_in *serv_addr) {
  //clean the serv_add structure
  memset( serv_addr , 0 , sizeof(struct sockaddr_in) );
  //internet family protocol
  serv_addr->sin_family = AF_INET ;
  //we bind to any ip form the host
  serv_addr->sin_addr.s_addr = htonl(INADDR_ANY) ;
  //we bind on the tcp port specified
  serv_addr->sin_port = htons(port) ;
}

////// bind///////

int do_bind(struct sockaddr_in serv_adr , int sockfd){
  int t = bind( sockfd , (struct sockaddr *)&serv_adr , sizeof(serv_adr) );
  if (t == -1) {
    ERROR_EXIT("bind");
  }
  else{
    return t;
  }
}

////// listen///////

int do_listen( int socket , int backlog){
  int t=listen( socket , backlog);
  if (t== -1) {
    ERROR_EXIT("listen");
  }
  else{
    return t;
  }
}

//////// accept//////

int do_accept(int sockfd , struct sockaddr* serv_adr , socklen_t* addrlen){
  int a=accept(sockfd , serv_adr , addrlen);
  if (a== -1) {
    ERROR_EXIT("accept");
  }
  else{
    return a;
  }
}


int do_read(int sockfd, char* buffer, int len){
  int t = 0;
  while (t != len) {
    read (sockfd, buffer + t, len - t);
    t+=1;
  }
  return t;
}
/////////////write//////
int do_write (int fd,const void *buffer, size_t count){
  int t = 0;
  while(t != count){
    write (fd, buffer + t, count - t);
    t+=1;
  }
  return t;
}
