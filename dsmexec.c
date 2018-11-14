#include "common_impl.h"
#include <stdio.h>


/* variables globales */
typedef int pipe_t[2];

/* un tableau gerant les infos d'identification */
/* des processus dsm */
dsm_proc_t *proc_array = NULL;

/* le nombre de processus effectivement crees */
volatile int num_procs_creat = 0;

void usage(void)
{
  fprintf(stdout,"Usage : dsmexec machine_file executable arg1 arg2 ...\n");
  fflush(stdout);
  exit(EXIT_FAILURE);
}

void sigchld_handler(int sig)
{
  /* on traite les fils qui se terminent */
  /* pour eviter les zombies */
}


int main(int argc, char *argv[])
{
  if (argc < 3){
    usage();
  } else {
    pid_t pid;
    int num_procs = 0;
    int i,j;
    int sockfd;
    int port;
    socklen_t addrlen;
    pipe_t * pipeout = NULL;
    pipe_t * pipeerr = NULL;



    /* Mise en place d'un traitant pour recuperer les fils zombies*/
    /* XXX.sa_handler = sigchld_handler; */

    /* lecture du fichier de machines */
    FILE *fichier=fopen("../machine_file", "r");
    /* 1- on recupere le nombre de processus a lancer */
    char machine[100];
    while (fgets(machine ,100 , fichier )!=NULL) {
      num_procs++;
    }
    fseek(fichier, 0, SEEK_SET);
    proc_array=(dsm_proc_t *)malloc(num_procs*sizeof(dsm_proc_t));

    /* 2- on recupere les noms des machines : le nom de */
    /* la machine est un des elements d'identification */
    for ( j = 0; j < num_procs; j++) {
      proc_array[j].connect_info.rank=j;
      fgets(proc_array[j].connect_info.hostname ,100 , fichier);
      int ss = strlen(proc_array[j].connect_info.hostname);
      proc_array[j].connect_info.hostname[ss-1]='\0';
    }

    /* creation de la socket d'ecoute */
    /* + ecoute effective */
    sockfd=creer_socket(SOCK_STREAM,&port);
    listen(sockfd , num_procs);



    /* creation des fils */
    pipeout = (pipe_t *) malloc(num_procs*sizeof(pipe_t));
    pipeerr = (pipe_t *) malloc(num_procs*sizeof(pipe_t));
    for ( i = 0; i < num_procs; i++) {
      /* creation du tube pour rediriger stdout */
      pipe(pipeout[i]);
    	/* creation du tube pour rediriger stderr */
      pipe(pipeerr[i]);

    }
    for(i = 0; i < num_procs ; i++) {

      pid = fork();
      if(pid == -1) ERROR_EXIT("fork");

      if (pid == 0) { /* fils */

        /* redirection stdout */
        close(pipeout[i][0]);
        dup2(pipeout[i][1], 1);

        /* redirection stderr */
        close(pipeerr[i][0]);
        dup2(pipeerr[i][1], 2);
        /* Creation du tableau d'arguments pour le ssh */
        int j;
        char * arguments[argc+4];
        arguments[0] = "ssh";
        arguments[1] = proc_array[i].connect_info.hostname;
        arguments[2] = "projet-system/Phase1/bin/dsmwrap";
        char hostname[20];
        gethostname(hostname,20);
        struct hostent *host_entry=NULL;
        host_entry=gethostbyname(hostname);
        arguments[3]=inet_ntoa(*(struct in_addr*)*host_entry->h_addr_list);


        char* add_port = malloc(5*sizeof(char));
        sprintf(add_port , "%d",port);
        arguments[4] = add_port;


        for (j = 2; j < argc; j++) {
          arguments[j+3] = argv[j];
        }
        arguments[argc+3] = NULL;
        /* jump to new prog : */
        execvp("ssh", arguments);

      } else  if(pid > 0) { /* pere */
        /* fermeture des extremites des tubes non utiles */
        char *msg=malloc(7*sizeof(char));
       close(pipeout[i][1]);
       close(pipeerr[i][1]);
       read(pipeout[i][0], msg,7*sizeof(char));
       printf("%s\n",msg );
       fflush(stdout);

        num_procs_creat++;
        free(msg);
      }
    }


    for(i = 0; i < num_procs ; i++){
      int n=0 ;

      /* on accepte les connexions des processus dsm */
      struct sockaddr sock_client;
      //accept connection from client
      socklen_t length= sizeof(sock_client);

      int sock_fils=do_accept( sockfd , &sock_client ,&length );
      proc_array[n].connect_info.fd=sock_fils;

      /*  On recupere le nom de la machine distante */
      /* 1- d'abord la taille de la chaine */
      char* taille=malloc(2*sizeof(char));
      do_read(sock_fils , taille , 2*sizeof(char) );
      int taille1=atoi(taille);
      /* 2- puis la chaine elle-meme */
      char* machine= malloc(taille1*sizeof(char));
      do_read(sock_fils , machine , taille1*sizeof(char));
      printf("%s\n",machine );
      while (strcmp(proc_array[n].connect_info.hostname,machine)!=0) {
        n++;
      }
      /* On recupere le pid du processus distant  */
      char* pid_c=malloc(6*sizeof(char));
      do_read(sock_fils , pid_c , 6*sizeof(char));
      proc_array[n].pid=(pid_t)(atoi(pid_c));

      /* On recupere le numero de port de la socket */
      /* d'ecoute des processus distants */
      char* port_c=malloc(6*sizeof(char));
      do_read(sock_fils , port_c , 6*sizeof(char));
      proc_array[n].connect_info.port=atoi(port_c);
    }
    for(i = 0; i < num_procs ; i++){

    /* envoi du nombre de processus aux processus dsm*/
    char* num_proc_c=malloc(2*sizeof(char));
    sprintf(num_proc_c , "%d" ,num_procs);
    do_write(proc_array[i].connect_info.fd ,num_proc_c,2*sizeof(char));
    /* envoi des rangs aux processus dsm*/
    char* rang_c=malloc(2*sizeof(char));
    sprintf(rang_c , "%d" ,proc_array[i].connect_info.rank);
    do_write(proc_array[i].connect_info.fd ,rang_c,2*sizeof(char));
    /* envoi des infos de connexion aux processus*/
    for (j = 0; j <num_procs ; j++) {
      char* port_c=malloc(6*sizeof(char));
      sprintf(port_c,"%d",proc_array[j].connect_info.port);
      do_write(proc_array[i].connect_info.fd ,proc_array[j].connect_info.hostname,strlen(proc_array[j].connect_info.hostname)*sizeof(char));
      do_write(proc_array[i].connect_info.fd ,port_c,strlen(port_c)*sizeof(char));
      free(port_c);
    }
}
    /* gestion des E/S : on recupere les caracteres */
    /* sur les tubes de redirection de stdout/stderr */
    /* while(1)
    {
    je recupere les infos sur les tubes de redirection
    jusqu'à ce qu'ils soient inactifs (ie fermes par les
    processus dsm ecrivains de l'autre cote ...)

  };
  */

  /* on attend les processus fils */

  /* on ferme les descripteurs proprement */

  /* on ferme la socket d'ecoute */
}
exit(EXIT_SUCCESS);
}
