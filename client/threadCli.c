
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include "funcCli.h"
#include "threadCli.h"
#include <unistd.h>

#define SIZE 1024

/**
 * @brief Thread that manages the reception of messages
 * 
 * @param datas Information to communicate with the server
 */
void * Receive(data* datas) {
  while (!datas->stop) {
    int size;
    int sizeRCV = recv(datas->dS, &size, sizeof(int), 0); // Receives the size of the message that will follow
    if (sizeRCV == -1) { perror("[-]Error recv"); shutdown(datas->dS, 2); exit(0);}
    else if (sizeRCV == 0) { break; }

    char *msg = (char *)malloc(sizeof(char)*size);
    int msgRCV = recv(datas->dS, msg, size*sizeof(char), 0); // Receives the message
    if (msgRCV == -1 ) { perror("[-]Error recv"); shutdown(datas->dS, 2); exit(0); }
    else if (msgRCV == 0) { break; }

    printf("%s\n\n", msg);
    free(msg);
  }
  pthread_exit(0);
}

/**
 * @brief // Sending a file to the server
 * 
 * @param sfiles File management structure
 */
void file(sfile* sfiles) { 
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

    if( send(sockfd,sfiles->filename,50,0) < 0) { perror("[-]Send failed"); exit(0); }
 
    fp = fopen(sfiles->filename, "r");
    if (fp == NULL) { perror("[-]Error in reading file"); exit(1);}
 
    send_file(fp, sockfd); // Send the file
    printf("\033[32;1;1m## File sent ##\033[0m\n\n");
 
    close(sockfd);
  } 
  else { printf("\033[32;1;1m## This file does not exist ##\033[0m\n\n"); }

  pthread_exit(0);
}

/**
 * @brief Receiving a file from the server
 * 
 * @param sfiles File management structure
 */
void downloadFile(sfile* sfiles) {
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

  if( send(sockfd,sfiles->filename,50,0) < 0) { perror("[-]Send failed"); exit(0); }

  write_file(sockfd,sfiles->filename); // Receive the file
 
  close(sockfd);
  pthread_exit(0);
}
