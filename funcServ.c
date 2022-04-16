
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <pthread.h>
#include "funcServ.h"

#define haveToStop "end"

void * receiveSend(data* datas) {

    int index = (*datas).indexCli-1;
    int stop = 0;
    
    while (!stop) {

        int taille;
        int tailleRCV = recv((*datas).arrayCli[index], &taille, sizeof(int), 0); // Receives the size of the message that will follow
        if (tailleRCV == -1) { perror("Error recv"); shutdown((*datas).arrayCli[index], 2); shutdown((*datas).dS, 2); exit(0); }
        else if (tailleRCV == 0) { break; }

        char *msg = (char *)malloc(sizeof(char)*taille);
        int msgRCV = recv((*datas).arrayCli[index], msg, taille*sizeof(char), 0); // Receives a message from a client
        if (msgRCV == -1 ) { perror("Error recv"); shutdown((*datas).arrayCli[index], 2); shutdown((*datas).dS, 2); exit(0); }
        else if (msgRCV == 0) { break; }
        printf("Message received : %s\n", msg) ;

        strtok(msg,"\n");
        strtok(haveToStop,"\0");
        if (strcmp(msg, haveToStop) == 0) { stop = 1; } // If the message sent is end

        for (int i=0;i<(*datas).indexCli;i++) {
            if ((*datas).arrayCli[index] != datas->arrayCli[i]) { // If the client is different from the one sending
                if (send(datas->arrayCli[i], &taille, sizeof(int), 0) == -1) { perror("Error send"); shutdown((*datas).arrayCli[index], 2); shutdown((*datas).dS,2); exit(0); }
            }
        }

        for (int i=0;i<(*datas).indexCli;i++) {
            if ((*datas).arrayCli[index] != datas->arrayCli[i]) { // If the client is different from the one sending
                if (send(datas->arrayCli[i], msg, taille*sizeof(char), 0) == -1) {  perror("Error send"); shutdown((*datas).arrayCli[index], 2); shutdown((*datas).dS, 2); exit(0); }
                printf("Message sent\n");
            }
        }

        free(msg);

  }

  shutdown((*datas).arrayCli[datas->indexCli],2); // Close the client

  if (index != 0) {
    for (int i=index;i<(datas->indexCli)-1;i++) {  // Change client numbers (currently not working as desired)
        (*datas).arrayCli[i] = (*datas).arrayCli[i+1]; 
    }
    (*datas).arrayCli[index]--;
  }

  pthread_exit(0);

}
