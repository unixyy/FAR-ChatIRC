
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <pthread.h>
#include "funcServ.h"

#define haveToStop "@quit"

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
        else if (isCommand(msg)) { 
            executeCommand(msg,datas,index);
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

int isCommand(char* msg) {
    if (msg[0] == '/') {
        return 1;
    }
    else {
        return 0;
    }
}

char** getCommand(char* msg) {
    char* command = (char*)malloc(sizeof(char)*strlen(msg));
    char * sauv = (char*)malloc(sizeof(char)*strlen(msg));
    strcpy(sauv,msg);
    strcpy(command,msg);
    char *datas[3];
    for (int i=0;i<3;i++) {
        datas[i]=(char *)malloc(sizeof(char) * 200);
    }

    char d[] = " ";
    char *p = strtok(msg, d);
    int i=0;
    while((p != NULL) && (i<2))
    {
        strcpy(datas[i],p);
        p = strtok(NULL, d);
        i++;
    }

    char* chaine = (char*)malloc(sizeof(char)*strlen(msg));

    int iterateur=0;
    int iterateur2 = 0;
    int iterateur3 = 0; 
    while(sauv[iterateur] != '\0') {
        if (iterateur2 >= 2) {
            chaine[iterateur3] = sauv[iterateur];
            iterateur3++;
        }
        if ((sauv[iterateur] == ' ') && (iterateur2 <= 1)) {
            iterateur2++;
        }
    iterateur++;
    }

    strcpy(datas[2],chaine);

    return datas;
}

void executeCommand(char* content, data* data, int id) {
    char ** command = getCommand(content);
    char * test2 = command[0];
    char test[] = "/test";
    strtok(test,"\0");
    strtok(test2,"\0");
    strtok(test2,"\n");
    if (strcmp(test2,"/help") == 0) { 
        char content[500] = "";
        char* test8 = helpMessage(&content);
        privateMessage(test8, data->arrayName[id], data,id);
    }
    else if (strcmp(test2,"/msg") == 0) {
        char * test3 = command[1];
        strtok(test3,"\0");
        char * test4 = command[2];
        strtok(test4,"\0");
        privateMessage(test4, test3, data,id);
    }
    else if (strcmp(test2,"/list") == 0) { 
        char content[500] = "";
        char* test8 = listClient(&content,data);
        privateMessage(test8, data->arrayName[id], data,id);
    }
    else {
        printf("Command not found\n");
        privateMessage(helpMessage(&content), data->arrayName[id], data,id);
    }
}

int nameToId(char* username, data* data) {
    for (int i=0;i<20;i++) {
        strtok(data->arrayName[i],"\n");
        strtok(data->arrayName[i],"\0");
        strtok(username,"\n");
        strtok(username,"\0");
        char * test = data->arrayName[i];
        strtok(test,"\n");
        strtok(test,"\0");
        if (strcmp(test,username) == 0) {
            return data->arrayId[i];
        }
    }
    return -1;
}

void privateMessage(char* msg, char* username, data* data, int index) {
    char* az = (char*)malloc(sizeof(char)*strlen(msg));
    strcpy(az,msg);
    int id = nameToId(username, data);
    if (id != -1) {
        int taille = 200;
        send(id, &taille, sizeof(int), 0);
        send(id, msg, taille*sizeof(char),0);
    }
    else {
        printf("error\n");
    }
}

char* helpMessage(char* content) {
    FILE *f;
    char c;
    f = fopen("listCommand.txt", "rt");
    int i = 0;
    while((c=fgetc(f))!=EOF){
        content[i] = c;
        i++;
    }
    fclose(f);
    return content;
}

char * listClient(char* content, data* data) { 
    int iterateur = 0;
    for (int i=0;i<20;i++) {
        if (data->arrayId[i] != -1) {
            int iterateur2 = 0;
            while((data->arrayName[i])[iterateur2] != '\0') {
                content[iterateur] = (data->arrayName[i])[iterateur2];
                iterateur++;
                iterateur2++;
            }
            content[iterateur] = ' ';
            iterateur++;
        }
    }
    return content;
}
