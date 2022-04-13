#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

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

  while (1) {

        struct sockaddr_in aC ;
    socklen_t lg = sizeof(struct sockaddr_in) ;
    int dSC = accept(dS, (struct sockaddr*) &aC,&lg) ;
    if (dSC == -1) {
      perror("Erreur accept");
    shutdown(dS, 2) ;
      exit(0);
    }
    printf("Client Connecté\n");

    int dSC2 = accept(dS, (struct sockaddr*) &aC,&lg) ;
    if (dSC2 == -1) {
      perror("Erreur accept");
    shutdown(dS, 2) ;
      exit(0);
    }
    printf("Client 2 Connecté\n");

  int stop = 0;
  char haveToStop[] = "fin";

  while (!stop) {


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
      stop=1;
    } 
    

    if (send(dSC2, &taille, sizeof(int), 0) == -1) {
      perror("Erreur send");
      shutdown(dSC, 2) ; 
       shutdown(dS,2) ;  
      exit(0);
    }
    printf("Taille Envoyé \n");
  
    if (send(dSC2, msg, taille*sizeof(char), 0) == -1) {
      perror("Erreur send");
         shutdown(dSC, 2) ; 
    shutdown(dS, 2) ;
      exit(0);
    }
    printf("Message Envoyé\n");
    free(msg);



  int taille2;
    int tailleRCV2 = recv(dSC2, &taille2, sizeof(int), 0);
    if (tailleRCV2 == -1) {
      perror("Erreur recv");
      shutdown(dSC2, 2) ; 
      shutdown(dS, 2) ;
      exit(0);
    }
    else if (tailleRCV2 == 0) {
      break;
    }
    printf("taille reçu : %d\n", taille2) ;

    char *msg2 = (char *)malloc(sizeof(char)*taille2);
    int msgRCV2 = recv(dSC2, msg2, taille2*sizeof(char), 0);
    if (msgRCV2 == -1 ) {
      perror("Erreur recv");
         shutdown(dSC2, 2) ; 
    shutdown(dS, 2) ;
      exit(0);
    }
    else if (msgRCV2 == 0) {
      break;
    }
    printf("Message reçu : %s\n", msg2) ;

    strtok(msg,"\n");
    strtok(haveToStop,"\0");
    if (strcmp(msg2, haveToStop) == 0) {
      stop=1;
    } 

    if (send(dSC, &taille2, sizeof(int), 0) == -1) {
      perror("Erreur send");
      shutdown(dSC, 2) ; 
       shutdown(dS,2) ;  
      exit(0);
    }
    printf("Taille Envoyé \n");
  
    if (send(dSC, msg2, taille2*sizeof(char), 0) == -1) {
      perror("Erreur send");
         shutdown(dSC, 2) ; 
    shutdown(dS, 2) ;
      exit(0);
    }
    printf("Message Envoyé\n");
    free(msg2);

  
  }
       shutdown(dSC, 2) ;
       shutdown(dSC2, 2) ;
  }
  shutdown(dS, 2) ;
  printf("Fin du programme");

  
}