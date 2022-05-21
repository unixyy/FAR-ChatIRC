
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
  int start = 1;
  while (1) {
    int taille2;
    int tailleRCV = recv(datas->dS, &taille2, sizeof(int), 0); // Receives the size of the message that will follow
    if (tailleRCV == -1) { perror("Error recv"); shutdown(datas->dS, 2); exit(0);}
    else if (tailleRCV == 0) { break; }

    char *msg = (char *)malloc(sizeof(char)*taille2);
    int msgRCV = recv(datas->dS, msg, taille2*sizeof(char), 0); // Receives the message that will follow
    if (msgRCV == -1 ) { perror("Error recv"); shutdown(datas->dS, 2); exit(0); }
    else if (msgRCV == 0) { break; }
    if (start) { 
      printf("\033[37;1;7m%s\n\033[0m",msg);
      start = 0;
    }
    else { 
      printf("%s\n", msg) ;
      //printf("\033[34;1;1m##########\n\033[0m");
      printf("\n");
    }

    free(msg);
  }
  datas->stop = 1;
  pthread_exit(0);
}

int isCommand(char* msg) { // Determines if the message contains a command
    if (msg[0] == '&') { return 1; }
    else { return 0; }
}

char** getCommand(char* msg) { // Separates the information in a message
    char * save = (char*)malloc(sizeof(char)*strlen(msg));
    strcpy(save,msg);
    char *datas[2];
    for (int i=0;i<2;i++) { datas[i]=(char *)malloc(sizeof(char) * 200);}

    char d[] = " ";
    char *p = strtok(msg, d);
    int i = 0;
    while((p != NULL) && (i<2)) // Recover the two parts
    {
        strcpy(datas[i],p);
        p = strtok(NULL, d);
        i++;
    }
    free(save);

    return datas;
}

void executeCommand(char* content, sfile* sfiles, char* ip) { // Executes a particular command
    char * save = (char*)malloc(sizeof(char)*50);
    char ** command = getCommand(content);
    char * toCompare = command[0];
    char* name = command[1];
    strcat(save,toCompare);
    strcat(save," ");
    strcat(save,name);
    strtok(toCompare,"\0");
    strtok(toCompare,"\n");
    if (strcmp(toCompare,"&files") == 0) { // List of personnal files
        listFile();
    }
    else if (strcmp(toCompare,"&up") == 0) { // Upload a file on the server
      pthread_t threadFile;
      sfiles->ip = ip;
      sfiles->filename = name;
      pthread_create(&threadFile, NULL, (void*)file, sfiles); // Creates a thread that manages the sending of a file
    }
    else if (strcmp(toCompare,"&dl") == 0) { // Download a file from the server
      pthread_t threadRFile;
      sfiles->ip = ip;
      sfiles->filename = name;
      pthread_create(&threadRFile, NULL, (void*)downloadFile, sfiles); // Creates a thread that manages the reciving of a file
    }
    free(toCompare);
    free(save);
}

int listFile() { // Print files list of current directory
    struct dirent *dir;
    DIR *d = opendir("."); 
    if (d) {
      while ((dir = readdir(d)) != NULL) { // There are some files
        printf("%s\n", dir->d_name);
      }
      closedir(d);
    }
    return 0;
}

void send_file(FILE *fp, int sockfd) { // Send the content of a file
  int n;
  char data[SIZE] = {0};
  while(fread(data, sizeof(char), SIZE, fp) != NULL) { // There are data to send
    if (send(sockfd, data, sizeof(data), 0) == -1) { perror("[-]Error in sending file."); exit(1);}
    bzero(data, SIZE);
  }
}
 
void file(sfile* sfiles) { // Sending a file to the server
  int stop = 0;
  strtok(sfiles->filename,"\n");  
  struct dirent *dir;
  DIR *d = opendir("."); 
  if (d)  {
    while ((dir = readdir(d)) != NULL) { // There are some files
      if (strcmp(sfiles->filename,dir->d_name) == 0) { stop=1; }
    }
    closedir(d);
  }  
  if (stop) { // Filename exists
    int port = 3030;
    int e;

    int sockfd;
    struct sockaddr_in server_addr;
    FILE *fp;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) { perror("[-]Error in socket"); exit(1);}
 
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(sfiles->ip);
 
    e = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)); // Connection to the socket

    while (e==-1) { e = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));}

    if( send(sockfd,sfiles->filename,50,0) < 0) { puts("Send failed"); exit(0); }
 
    fp = fopen(sfiles->filename, "r");
    if (fp == NULL) { perror("[-]Error in reading file."); exit(1);}
 
    send_file(fp, sockfd); // Send the file
    printf("File sent\n");
 
    close(sockfd);
  } else { printf("This file does not exist.\n"); }
  pthread_exit(0);
}

void write_file(int sockfd, char* filename) { // Create the new file
  int n;
  FILE *fp;
  char buffer[SIZE];
  char * path = malloc(50*sizeof(char));
  strcat(path,"./");
  strcat(path,filename);

  n = recv(sockfd, buffer, SIZE, 0);
  
  if (strlen(buffer)==0) { printf("This file does not exist.\n");}
  else { fp = fopen(filename, "w"); fputs(buffer,fp); bzero(buffer, SIZE); printf("File received.\n");}
  
  fclose(fp);
  return;
}
 
void downloadFile(sfile* sfiles) { // Receiving a file from the server
  int port = 3033;
  int e;
  strtok(sfiles->filename,"\n");
 
  int sockfd, new_sock;
  struct sockaddr_in server_addr;
  FILE *fp;
  socklen_t addr_size;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0) { perror("[-]Error in socket"); exit(1); }
 
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = port;
  server_addr.sin_addr.s_addr = inet_addr(sfiles->ip);
 
  e = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)); // Connection to the socket

  while (e==-1) { e = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)); }

  if( send(sockfd,sfiles->filename,50,0) < 0) { puts("Send failed"); exit(0); }

  write_file(sockfd,sfiles->filename); // Receive the file
 
  close(sockfd);
  pthread_exit(0);
}
