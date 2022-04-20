
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
    printf("ok\n");
    int index = actualIndex(&datas);
    printf("l'index est %d\n",index);
    //printf(datas->arrayCli[index][0]);
    int stop = 0;
    int firstmsg = 1;

    while (!stop) {

        int taille;
        //printf("%d\n",(int)datas->arrayCli[index][0]);
        int tailleRCV = recv(4, &taille, sizeof(int), 0); // Receives the size of the message that will follow
        if (tailleRCV == -1) { perror("Error recv 1 "); exit(0); }
        else if (tailleRCV == 0) { break; }

        printf("taille received\n");

        char *msg = (char *)malloc(sizeof(char)*taille);
        int msgRCV = recv((int)datas->arrayCli[index][0], msg, taille*sizeof(char), 0); // Receives a message from a client
        if (msgRCV == -1 ) { perror("Error recv 2 "); shutdown(index, 2); shutdown((*datas).dS, 2); exit(0); }
        else if (msgRCV == 0) { break; }
        printf("Message received : %s\n", msg) ;
        if(firstmsg == 1){
            datas->arrayCli[index][1] = msg;
            firstmsg=9999;
        }
        else {
        strtok(msg,"\n");
        strtok(haveToStop,"\0");
        if (strcmp(msg, haveToStop) == 0) { stop = 1; } // If the message sent is end

        for (int i=0;i<20;i++) {
            if (((int)datas->arrayCli[index][0] != (int)datas->arrayCli[i][0]) && ((int)datas->arrayCli[i][0] != "")) { // If the client is different from the one sending
                if (send((int)datas->arrayCli[i][0], &taille, sizeof(int), 0) == -1) { perror("Error send"); shutdown((int)datas->arrayCli[index][0], 2); shutdown((*datas).dS,2); exit(0); }
            }
        }

        for (int i=0;i<20;i++) {
            if (((int)datas->arrayCli[index][0] != (int)datas->arrayCli[i][0]) && ((int)datas->arrayCli[i][0] != "")) { // If the client is different from the one sending
                if (send((int)datas->arrayCli[i][0], msg, taille*sizeof(char), 0) == -1) {  perror("Error send"); shutdown((int)datas->arrayCli[index][0], 2); shutdown((*datas).dS, 2); exit(0); }
                printf("Message sent\n");
            }
        }
        }

        free(msg);

  }

  shutdown((*datas).arrayCli[datas->indexCli],2); // Close the client

  deleteUser(&datas,(char*)(int)datas->arrayCli[index][0]);

  pthread_exit(0);

}

int actualIndex(data* data) {
    int cpt = 0;
    int stop = 0;
    while (!stop) { 
        if ((int)data->arrayCli[cpt][0] == data->id) {
            stop = 1;
        }
        cpt++;
    }
    return cpt-2;
}

int nextEmpty(data* data) {
    for (int i=0;i<20;i++) {
        if (strcmp(data->arrayCli[i][0],"nonEmpty") == 0) {
            printf("%d\n", i);
            return i;
        }
    }
    return -1;
}

void deleteUser(data* data, char* id) {
    for (int i=0;i<20;i++) {
        if (data->arrayCli[i][0] == id) {
            data->arrayCli[i][0] = "nonEmpty";
            data->arrayCli[i][1] = "nonEmpty";
            data->indexCli--;
            return;
        }
    }
}
