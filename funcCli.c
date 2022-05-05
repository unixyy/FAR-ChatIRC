
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include "funcCli.h"
#include <unistd.h>

#define SIZE 1024

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

int isCommand(char* msg) { // Determines if the received message contains a command
    if (msg[0] == '&') {
        return 1;
    }
    else {
        return 0;
    }
}

char** getCommand(char* msg) { // Separates the information in a message
    char * save = (char*)malloc(sizeof(char)*strlen(msg));
    strcpy(save,msg);
    char *datas[2];
    for (int i=0;i<2;i++) {
        datas[i]=(char *)malloc(sizeof(char) * 200);
    }

    char d[] = " ";
    char *p = strtok(msg, d);
    int i = 0;
    while((p != NULL) && (i<2)) // Recover the first two parts
    {
        strcpy(datas[i],p);
        p = strtok(NULL, d);
        i++;
    }
    free(save);

    return datas;
}

void executeCommand(char* content, sfile* sfiles, char* ip) {
    char * save = (char*)malloc(sizeof(char)*50);
    char ** command = getCommand(content);
    char * toCompare = command[0];
    char* name = command[1];
    strcat(save,toCompare);
    strcat(save," ");
    strcat(save,name);
    strtok(toCompare,"\0");
    strtok(toCompare,"\n");
    if (strcmp(toCompare,"&pfile") == 0) { // Help command
        listFile();
    }
    else if (strcmp(toCompare,"&send") == 0) { // Files list of the server
      pthread_t threadFile;
      sfiles->ip = ip;
      sfiles->filename = name;
      int taille = strlen(save);
      if (send(datas.dS, &taille, sizeof(int), 0) == -1) { perror("Error send"); exit(0); } // Sends the message size to the server
      if (send(datas.dS, save, strlen(save) , 0) == -1) { perror("Error send"); exit(0); } // Sends the message to the server
      sleep(1);
      pthread_create(&threadFile, NULL, (void*)file, sfiles); // Creates a thread that manages the reciving of messages
    }
    free(toCompare);
    free(save);
}

int listFile() { // Print files list of current directory
    struct dirent *dir;
    DIR *d = opendir("."); 
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            printf("%s\n", dir->d_name);
        }
        closedir(d);
    }
    return 0;
}

void send_file(FILE *fp, int sockfd){
  int n;
  char data[SIZE] = {0};
 
  while(fgets(data, SIZE, fp) != NULL) {
    if (send(sockfd, data, sizeof(data), 0) == -1) {
      perror("[-]Error in sending file.");
      exit(1);
    }
    bzero(data, SIZE);
  }
}
 
void file(sfile* sfiles){
  int port = 3030;
  int e;

  strtok(sfiles->filename,"\n");
 
  int sockfd;
  struct sockaddr_in server_addr;
  FILE *fp;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0) {
    perror("[-]Error in socket");
    exit(1);
  }
  //printf("[+]Server socket created successfully.\n");
 
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = port;
  server_addr.sin_addr.s_addr = inet_addr(sfiles->ip);
 
  e = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));

  while (e==-1) { 
    e = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
  }
 //printf("[+]Connected to Server.\n");
 
  fp = fopen(sfiles->filename, "r");
  if (fp == NULL) {
    perror("[-]Error in reading file.");
    exit(1);
  }
 
  send_file(fp, sockfd);
  //printf("[+]File data sent successfully.\n");
  printf("File send.\n");
 
  //printf("[+]Closing the connection.\n");
  close(sockfd);
 
  pthread_exit(0);

}