
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

/**
 * @brief Thread that manages the reception of messages
 * 
 * @param datas Information to communicate with the server
 */
void * Receive(data* datas) {
  int start = 1;
  while (1) {
    int size;
    int sizeRCV = recv(datas->dS, &size, sizeof(int), 0); // Receives the size of the message that will follow
    if (sizeRCV == -1) { perror("[-]Error recv"); shutdown(datas->dS, 2); exit(0);}
    else if (sizeRCV == 0) { break; }

    char *msg = (char *)malloc(sizeof(char)*size);
    int msgRCV = recv(datas->dS, msg, size*sizeof(char), 0); // Receives the message
    if (msgRCV == -1 ) { perror("[-]Error recv"); shutdown(datas->dS, 2); exit(0); }
    else if (msgRCV == 0) { break; }

    if (start) { printf("\033[37;1;7m%s\n\033[0m",msg); start = 0; } // To display the welcome message differently
    else { printf("%s\n\n", msg); printf("\n");}
    free(msg);
  }
  datas->stop = 1;
  pthread_exit(0);
}

/**
 * @brief Determines if the message contains a command
 * 
 * @param msg the message received
 * @return 1 if the message contains a command, 0 otherwise
 */
int isCommand(char* msg) { // Determines if the message contains a command
    if (msg[0] == '&') { return 1; }
    else { return 0; }
}

/**
 * @brief Separates the information in a message
 * 
 * @param msg The message to check
 * @return The separated content 
 */
char** getCommand(char* msg) {
    char * save = (char*)malloc(sizeof(char)*strlen(msg));
    strcpy(save,msg);
    char *datas[2];
    for (int i=0;i<2;i++) { datas[i]=(char *)malloc(sizeof(char) * 200); } // Initialize the returned array

    char d[] = " ";
    char *p = strtok(msg, d);
    int i = 0;
    while((p != NULL) && (i<2)) { // Recover the two parts of the command
        strcpy(datas[i],p);
        p = strtok(NULL, d);
        i++;
    }
    free(save);
    return datas;
}

/**
 * @brief Executes a particular command
 * 
 * @param content Message containing the command
 * @param sfiles File management structure
 * @param ip Server ip address
 */
void executeCommand(char* content, sfile* sfiles, char* ip) {
    char * save = (char*)malloc(sizeof(char)*100);
    char ** command = getCommand(content);
    char * toCompare = command[0];
    char* name = command[1];
    strcat(save,toCompare);
    strcat(save," ");
    strcat(save,name);
    strtok(toCompare,"\0");
    strtok(toCompare,"\n");
    if (strcmp(toCompare,"&files") == 0) { listFile(); } // List of personnal files
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

/**
 * @brief // Print files list of current directory
 * 
 * @return A default 0
 */
int listFile() { 
    struct dirent *dir;
    DIR *d = opendir("."); 
    if (d) {
      while ((dir = readdir(d)) != NULL) { printf("%s\n", dir->d_name); } // There are some files
      closedir(d);
    }
    return 0;
}

/**
 * @brief // Send the content of a file
 * 
 * @param fp Name of the file
 * @param sockfd Transmission socket number
 */
void send_file(FILE *fp, int sockfd) { 
  int n;
  char data[SIZE] = {0};
  while(fread(data, sizeof(char), SIZE, fp) != (unsigned long) NULL) { // There are data to send
    if (send(sockfd, data, sizeof(data), 0) == -1) { perror("[-]Error in sending file"); exit(1);}
    bzero(data, SIZE);
  }
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
    printf("## File sent ##\n\n");
 
    close(sockfd);
  } 
  else { printf("## This file does not exist ##\n"); }

  pthread_exit(0);
}

/**
 * @brief Create a new file with a content
 * 
 * @param sockfd Transmission socket number
 * @param filename Name of the file
 */
void write_file(int sockfd, char* filename) {
  int n;
  FILE *fp;
  char buffer[SIZE];
  char * path = malloc(100*sizeof(char));
  strcat(path,"./");
  strcat(path,filename);

  n = recv(sockfd, buffer, SIZE, 0);
  
  if (strlen(buffer)==0) { printf("## This file does not exist ##\n");}
  else { fp = fopen(filename, "w"); fputs(buffer,fp); bzero(buffer, SIZE); printf("## File received ##\n\n");}
  
  fclose(fp);
  return;
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
