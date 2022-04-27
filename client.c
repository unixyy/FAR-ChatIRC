
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "funcCli.h"

#define haveToStop "@quit"

int main(int argc, char *argv[]) {

  int dS = socket(PF_INET, SOCK_STREAM, 0); // Create a socket
  if (dS == -1) { perror("Error socket"); exit(0); }

  struct sockaddr_in aS;
  aS.sin_family = AF_INET;
  inet_pton(AF_INET,argv[1],&(aS.sin_addr)); // Server ip adress
  aS.sin_port = htons(atoi(argv[2])); // Server port
  socklen_t lgA = sizeof(struct sockaddr_in);

  if (connect(dS, (struct sockaddr *) &aS, lgA) == -1) { perror("Error connect"); shutdown(dS,2); exit(0); } // Connection to the server

  int stop = 0;

  pthread_t my_thread;
  pthread_create(&my_thread, NULL, Receive, &dS); // Creates a thread that manages the reciving of messages

  printf("Bienvenue ! Vous pouvez chatter librement après avoir indiqué votre pseudo (/help en cas de besoin).\n");

  while (!stop) {

    char *m = (char *)malloc(sizeof(char)*30);
    fgets(m, sizeof(char)*30, stdin); // Message to send

    int taille = strlen(m);
    if (send(dS, &taille, sizeof(int), 0) == -1) { perror("Error send"); shutdown(dS,2); exit(0); } // Sends the message size to the server

    if (send(dS, m, strlen(m) , 0) == -1) { perror("Error send"); shutdown(dS,2); exit(0); } // Sends the message to the server

    strtok(m,"\n");
    strtok(haveToStop,"\0");
    if (strcmp(m, haveToStop) == 0) { stop=1; } 
  
    free(m);

  }

}
