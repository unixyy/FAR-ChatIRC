
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
    static char *datas[2];
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
