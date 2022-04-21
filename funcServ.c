
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

    int index = actualIndex(datas);
    int stop = 0;
    int firstmsg = 1;

    while (!stop) {

        int taille;
        int tailleRCV = recv(datas->arrayId[index], &taille, sizeof(int), 0); // Receives the size of the message that will follow
        if (tailleRCV == -1) { perror("Error recv 1 "); shutdown(index, 2); shutdown((*datas).dS, 2); exit(0); }
        else if (tailleRCV == 0) { break; }

        char *msg = (char *)malloc(sizeof(char)*taille);
        int msgRCV = recv(datas->arrayId[index], msg, taille*sizeof(char), 0); // Receives a message from a client
        if (msgRCV == -1 ) { perror("Error recv 2 "); shutdown(index, 2); shutdown((*datas).dS, 2); exit(0); }
        else if (msgRCV == 0) { break; }
        printf("Message received : %s\n", msg) ;

        if(firstmsg == 1){
            strcpy(datas->arrayName[index],msg);
            firstmsg=0;
        }
        else {
        
        strtok(msg,"\n");
        strtok(haveToStop,"\0");
        if (strcmp(msg, haveToStop) == 0) { stop = 1; } // If the message sent is end
            for (int i=0;i<20;i++) {
                if ((datas->arrayId[index] != datas->arrayId[i]) && (datas->arrayId[i] != -1)) { // If the client is different from the one sending
                    if (send(datas->arrayId[i], &taille, sizeof(int), 0) == -1) { perror("Error send"); shutdown(datas->arrayId[index], 2); shutdown((*datas).dS,2); exit(0); }
                }
            }

            for (int i=0;i<20;i++) {
                if ((datas->arrayId[index] != datas->arrayId[i]) && (datas->arrayId[i] != -1)) { // If the client is different from the one sending
                    if (send(datas->arrayId[i], msg, taille*sizeof(char), 0) == -1) {  perror("Error send"); shutdown(datas->arrayId[index], 2); shutdown((*datas).dS, 2); exit(0); }
                    printf("Message sent\n");
                }
            }
        }

        free(msg);

  }

  shutdown(datas->arrayId[index],2); // Close the client
  deleteUser(datas,datas->arrayId[index]);
  pthread_exit(0);

}

int actualIndex(data* data) {
    int cpt = 0;
    int stop = 0;
    while (!stop) { 
        if (data->arrayId[cpt] == data->actualId) {
            stop = 1;
        }
        cpt++;
    }
    return cpt-1;
}

int nextEmpty(data* data) {
    for (int i=0;i<20;i++) {
        if (data->arrayId[i] == -1) {
            return i;
        }
    }
    return -1;
}

void deleteUser(data* data, int id) {
    for (int i=0;i<20;i++) {
        if (data->arrayId[i] == id) {
            data->arrayId[i] = -1;
            strcpy(data->arrayName[i],"empty");
            data->numberCli--;
            return;
        }
    }
}
