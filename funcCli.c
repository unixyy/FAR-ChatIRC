
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "funcCli.h"

void * Receive(data* datas) { // Thread for receiving a message
  while (1) {
    int taille2;
    int tailleRCV = recv(datas->dS, &taille2, sizeof(int), 0); // Receives the size of the message that will follow
    if (tailleRCV == -1) { perror("Error recv"); shutdown(datas->dS, 2); exit(0);}
    else if (tailleRCV == 0) { break; }

    char *msg = (char *)malloc(sizeof(char)*taille2);
    int msgRCV = recv(datas->dS, msg, taille2*sizeof(char), 0); // Receives the message that will follow
    if (msgRCV == -1 ) { perror("Error recv"); shutdown(datas->dS, 2); exit(0); }
    else if (msgRCV == 0) { break; }
    printf("%s\n", msg) ;

    free(msg);
  }
  datas->stop = 1;
  pthread_exit(0);
}
