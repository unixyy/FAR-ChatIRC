#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <pthread.h>

#define NB_THREADS 2

int stop = 0;

void * receiveSend(void * dS, void * dSC){
    char haveToStop[] = "fin";
  while(1){
      int taille;
      int tailleRCV = recv(dSC, &taille, sizeof(int), 0);
      if (tailleRCV == -1) {
        perror("Erreur recv");
        shutdown(dSC, 2) ; 
        shutdown(dS, 2) ;
        exit(0);
      }
      else if (tailleRCV == 0) {
        break;
      }
      printf("taille reçu : %d\n", taille) ;

      char *msg = (char *)malloc(sizeof(char)*taille);
      int msgRCV = recv(dSC, msg, taille*sizeof(char), 0);
      if (msgRCV == -1 ) {
        perror("Erreur recv");
          shutdown(dSC, 2) ; 
      shutdown(dS, 2) ;
        exit(0);
      }
      else if (msgRCV == 0) {
        break;
      }
      printf("Message reçu : %s\n", msg) ;

      strtok(msg,"\n");
      strtok(haveToStop,"\0");
      if (strcmp(msg, haveToStop) == 0) {
        int tmp = stop;
        tmp = 1;
        stop = tmp;
      }

      if (send(dS, &taille, sizeof(int), 0) == -1) {
        perror("Erreur send");
        shutdown(dSC, 2) ; 
        shutdown(dS,2) ;  
        exit(0);
      }
      printf("Taille Envoyé \n");
    
      if (send(dS, msg, taille*sizeof(char), 0) == -1) {
        perror("Erreur send");
          shutdown(dSC, 2) ; 
      shutdown(dS, 2) ;
        exit(0);
      }
      printf("Message Envoyé\n");
      free(msg);
  }
}





int main(int argc, char *argv[]) {
  
  printf("Début programme\n");

  int dS = socket(PF_INET, SOCK_STREAM, 0);
  if (dS == -1) {
    perror("Erreur socket");
    return -1;
  }
  printf("Socket Créé\n");


  struct sockaddr_in ad;
  ad.sin_family = AF_INET;
  ad.sin_addr.s_addr = INADDR_ANY ;
  ad.sin_port = htons(atoi(argv[1])) ;
  if (bind(dS, (struct sockaddr*)&ad, sizeof(ad)) == -1){
    perror("Erreur bind");
    return -1;
  }
  printf("Socket Nommé\n");

  listen(dS, 7) ;
  printf("Mode écoute\n");
  int *CliArray = calloc(20, sizeof(int));
  int indexCli = 0;
  while (1)
  {

    struct sockaddr_in aC;
    socklen_t lg = sizeof(struct sockaddr_in) ;
    if (indexCli >= 19){
      printf("Trop de clients");
      sleep(5);
    }else{
      int dSC = accept(dS, (struct sockaddr*) &aC,&lg) ;
      if (dSC == -1) {
        perror("Erreur accept");
      shutdown(dS, 2) ;
        exit(0);
      }else{
        indexCli++;
      }
      printf("Client Connecté\n");
      CliArray[indexCli] = dSC;
  
    }
    int i = 0;

    pthread_t thread[NB_THREADS];
    for (i = 0; i < NB_THREADS; i++)
    {
      pthread_create(&thread[i], NULL, receiveSend,(void *)dSC);
    }
    for (i = 0; i < NB_THREADS; i++)
    {
      pthread_join(thread[i], NULL);
    }
  }
       shutdown(dSC, 2) ;
       shutdown(dSC2, 2) ;

  shutdown(dS, 2) ;
  printf("Fin du programme");

  
}