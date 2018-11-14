#include "common_impl.h"

int main(int argc, char **argv)
{
  /* processus intermediaire pour "nettoyer" */
  /* la liste des arguments qu'on va passer */
  /* a la commande a executer vraiment */
  int i;
  int port_dsm;
  int num_procs;
  int sock,sock_dsm;
  struct sockaddr_in addr_serv;
  char * arguments[argc-2];
  for (i = 3; i < argc; i++) {
    arguments[i-3] = argv[i];
  }
  arguments[argc-3] = NULL;

  /* creation d'une socket pour se connecter au */
  /* au lanceur et envoyer/recevoir les infos */
  /* necessaires pour la phase dsm_init */
  sock = do_socket(AF_INET,SOCK_STREAM , 0);
  inet_aton(argv[1],&addr_serv.sin_addr);
  addr_serv.sin_port=htons(atoi(argv[2]));
  addr_serv.sin_family = AF_INET;

  int s=connect(sock , (struct sockaddr* )&addr_serv , sizeof(struct sockaddr_in));
  if (s==-1) {
    perror("connect");
  }
  /* Envoi du nom de machine au lanceur*/
  char * ms = malloc(20*sizeof(char));
  gethostname(ms, 20);
  char* taille=malloc(2*sizeof(char));
  sprintf(taille,"%d",(int)strlen(ms));
  do_write(sock , taille ,2*sizeof(char));
  do_write(sock ,ms , strlen(ms)*sizeof(char));

  /* Envoi du pid au lanceur*/
  pid_t pid;
  pid= getpid();
  char* pid_c=malloc(6*sizeof(char));
  sprintf(pid_c , "%d" , (int)pid);
  do_write(sock , pid_c ,(strlen(pid_c)+1)*sizeof(char)+1);
  /* Creation de la socket d'ecoute pour les */
  /* connexions avec les autres processus dsm */
  sock_dsm=creer_socket(SOCK_STREAM,&port_dsm);

  /* Envoi du numero de port au lanceur */
  /* pour qu'il le propage à tous les autres */
  /* processus dsm */
  char* port_c=malloc(6*sizeof(char));
  sprintf(port_c , "%d" , port_dsm);
  do_write(sock , port_c ,(strlen(pid_c)+1)*sizeof(char)+1);

  /*On Récupère le nombre de processus dsm*/
  //char* num_c=malloc(2*sizeof(char));
  //do_read(sock,num_c,2*sizeof(char));
  //printf("%s\n",num_c );

  /* on execute la bonne commande */
  return 0;
}
