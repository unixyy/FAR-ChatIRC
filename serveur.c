#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <pthread.h>

#define NB_THREADS 2

struct data {
    int dS;
    int arrayCli[20];
    int indexCli;
  }datas;


typedef struct data data;

void * receiveSend(data* datas){
    char haveToStop[] = "fin";
    int index = (*datas).indexCli-1;
    int stop = 0;
  while(!stop){
      int taille;
      int tailleRCV = recv((*datas).arrayCli[index], &taille, sizeof(int), 0);
      if (tailleRCV == -1) {
        perror("Erreur recv");
        shutdown((*datas).arrayCli[index], 2) ; 
        shutdown((*datas).dS, 2) ;
        exit(0);
      }
      else if (tailleRCV == 0) {
        break;
      }
      // printf("taille reçu : %d\n", taille) ;

      char *msg = (char *)malloc(sizeof(char)*taille);
      int msgRCV = recv((*datas).arrayCli[index], msg, taille*sizeof(char), 0);
      if (msgRCV == -1 ) {
        perror("Erreur recv");
          shutdown((*datas).arrayCli[index], 2) ; 
      shutdown((*datas).dS, 2) ;
        exit(0);
      }
      else if (msgRCV == 0) {
        break;
      }
      printf("Message reçu : %s\n", msg) ;

      strtok(msg,"\n");
      strtok(haveToStop,"\0");
      if (strcmp(msg, haveToStop) == 0) {
        stop = 1;
      }

      for (int i=0;i<(*datas).indexCli;i++) {
        if ((*datas).arrayCli[index] != datas->arrayCli[i]) {
        if (send(datas->arrayCli[i], &taille, sizeof(int), 0) == -1) {
        perror("Erreur send");
        shutdown((*datas).arrayCli[index], 2) ; 
        shutdown((*datas).dS,2) ;  
        exit(0);
      }
      }
      // printf("Taille Envoyé \n");

      }

      
    for (int i=0;i<(*datas).indexCli;i++) {
      if ((*datas).arrayCli[index] != datas->arrayCli[i]) {
      if (send(datas->arrayCli[i], msg, taille*sizeof(char), 0) == -1) {
        perror("Erreur send");
          shutdown((*datas).arrayCli[index], 2) ; 
      shutdown((*datas).dS, 2) ;
        exit(0);
      }
      printf("Message Envoyé\n");
      }
  }
  free(msg);
  }
  shutdown((*datas).arrayCli[datas->indexCli],2);
  if (index != 0) {
    for (int i=index;i<(datas->indexCli)-1;i++) {
    (*datas).arrayCli[i] = (*datas).arrayCli[i+1];
  }
  (*datas).arrayCli[index]--;
  }
  pthread_exit(0);
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

  

  datas.dS = dS;
  datas.indexCli = 0;

    struct sockaddr_in aC;
    socklen_t lg = sizeof(struct sockaddr_in) ;
  while (1)
  {
    if (datas.indexCli >= 19){
      printf("Trop de clients");
      sleep(5);
    }else{
      datas.arrayCli[datas.indexCli] = accept(dS, (struct sockaddr*) &aC,&lg) ;
      if (datas.arrayCli[datas.indexCli] == -1) {
        perror("Erreur accept");
      shutdown(dS, 2) ;
        exit(0);
      }else{
        datas.indexCli++;
      }
      printf("Client Connecté\n");
  
    }

    pthread_t thread[NB_THREADS];
    pthread_create(&thread[datas.indexCli], NULL, receiveSend, &datas);

  }
  shutdown(datas.dS, 2) ;
  printf("Fin du programme");

  
}