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
        struct sockaddr_in aS;
    aS.sin_family = AF_INET;
    inet_pton(AF_INET,argv[1],&(aS.sin_addr)) ;
    aS.sin_port = htons(atoi(argv[2])) ;
    socklen_t lgA = sizeof(struct sockaddr_in) ;
    if (connect(dS, (struct sockaddr *) &aS, lgA) == -1) {
      perror("Erreur connect");
       shutdown(dS,2) ;  
      return -1;
    }

  int stop = 0;
  char haveToStop[] = "fin";

  while (!stop) {

    printf("Socket Connecté\n");

    int taille2;
    int tailleRCV = recv(dS, &taille2, sizeof(int), 0);
    if (tailleRCV == -1) {
      perror("Erreur recv");
      shutdown(dS, 2) ; 
      exit(0);
    }
    else if (tailleRCV == 0) {
      break;
    }
    printf("taille reçu : %d\n", taille2) ;

    char *msg = (char *)malloc(sizeof(char)*taille2);
    int msgRCV = recv(dS, msg, taille2*sizeof(char), 0);
    if (msgRCV == -1 ) {
      perror("Erreur recv");
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

    char *m = (char *)malloc(sizeof(char)*30);
    
    printf("Quel est votre nom ? ");
    fgets(m, sizeof(char)*30, stdin);

    int taille = strlen(m);
    if (send(dS, &taille, sizeof(int), 0) == -1) {
      perror("Erreur send");
       shutdown(dS,2) ;  
      exit(0);
    }
    printf("Taille Envoyé \n");

    if (send(dS, m, strlen(m) , 0) == -1) {
      perror("Erreur send");
       shutdown(dS,2) ;  
      exit(0);
    }
    printf("Message Envoyé \n");

strtok(m,"\n");
  if (strcmp(m, haveToStop) == 0) {
      stop=1;
    } 

  free(m);
  free(msg);

  }
      shutdown(dS,2) ;  
    printf("Fin du programme");
}