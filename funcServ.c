
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <pthread.h>
#include <dirent.h>
#include "funcServ.h"
#include <sys/types.h>
#include <sys/stat.h>

#define SIZE 1024
#define haveToStop "@quit"

void * receiveSend(data* datas, pthread_mutex_t* mutex) { // Thread for receiving and sending a message
    int index = actualIndex(datas);

    int stop = 0;
    int firstmsg = 1;

    int sizeWelcome = 77;
    if (send(datas->arrayId[index], &sizeWelcome, sizeof(int), 0) == -1) { perror("Error send"); shutdown(datas->arrayId[index], 2); shutdown((*datas).dS,2); exit(0); }
    if (send(datas->arrayId[index], "Welcome! You can chat freely after entering your nickname (/help if needed).", sizeWelcome*sizeof(char), 0) == -1) { perror("Error send"); shutdown(datas->arrayId[index], 2); shutdown((*datas).dS,2); exit(0); }
    
    while (!stop) {
        int size;
        int sizeRCV= recv(datas->arrayId[index], &size, sizeof(int), 0); // Receives the size of the message that will follow
        if (sizeRCV== -1) { perror("Error recv 1 "); shutdown(index, 2); shutdown((*datas).dS, 2); exit(0); }
        else if (sizeRCV== 0) { break; }

        char *msg = (char *)malloc(sizeof(char)*size);
        int msgRCV = recv(datas->arrayId[index], msg, size*sizeof(char), 0); // Receives a message from a client
        if (msgRCV == -1 ) { perror("Error recv 2 "); shutdown(index, 2); shutdown((*datas).dS, 2); exit(0); }
        else if (msgRCV == 0) { break; }
        printf("Message received : %s\n", msg) ;

        if(firstmsg == 1) { // First message sent
            if (checkPseudo(datas,msg) == 1) { // Nickname already used
                int sizeMsgUsername = 50;
                if (send(datas->arrayId[index], &sizeMsgUsername, sizeof(int), 0) == -1) { perror("Error send"); shutdown(datas->arrayId[index], 2); shutdown((*datas).dS,2); exit(0); }
                if (send(datas->arrayId[index], "Nickname already taken, please choose another one", sizeMsgUsername*sizeof(char), 0) == -1) { perror("Error send"); shutdown(datas->arrayId[index], 2); shutdown((*datas).dS,2); exit(0); }
            } else {
                pthread_mutex_lock(&mutex);
                strtok(msg,"\n");
                strcpy(datas->arrayName[index],msg);
                pthread_mutex_unlock(&mutex);
                firstmsg=0;
            }
        }
        else if (isCommand(msg)) { // command in the message
            executeCommand(msg,datas,index);
        }
        else {
            char *sender = calloc(30, sizeof(char));
            strcpy(sender,idToName(datas->arrayId[index], datas));
            strtok(sender, "\n");
            size = size + 3 + strlen(sender);
            char* content = (char *)malloc(sizeof(char)*size);
            strcpy(content,"");
            strcat(content, sender);
            strcat(content, " : ");
            strcat(content, msg);
            strtok(msg, "\n");
            strtok(haveToStop, "\0");
            if (strcmp(msg, haveToStop) == 0) { // If the message sent is @quit
                stop = 1; 
            } 
            else {
                for (int i=0;i<20;i++) {
                    if ((datas->arrayId[index] != datas->arrayId[i]) && (datas->arrayId[i] != -1)) { // If the client is different from the one sending
                        if (send(datas->arrayId[i], &size, sizeof(int), 0) == -1) { perror("Error send"); shutdown(datas->arrayId[index], 2); shutdown((*datas).dS,2); exit(0); }
                    }
                }
                for (int i=0;i<20;i++) {
                    if ((datas->arrayId[index] != datas->arrayId[i]) && (datas->arrayId[i] != -1)) { // If the client is different from the one sending
                        if (send(datas->arrayId[i], content, size*sizeof(char), 0) == -1) {  perror("Error send"); shutdown(datas->arrayId[index], 2); shutdown((*datas).dS, 2); exit(0); }
                        printf("Message sent\n");
                    }
                }
            }
        }

        free(msg);
  }
  shutdown(datas->arrayId[index],2); // Close the client
  pthread_mutex_lock(&mutex);
  deleteUser(datas,datas->arrayId[index]);
  datas->threadToClose[index] = pthread_self();
  datas->isClose[index] = 1;
  pthread_mutex_unlock(&mutex);
  rk_sema_post(datas->s);
  pthread_exit(0);
}

void * closeThread(data* datas) { // Thread for closing a receiveSend thread
    while(1) { 
        for (int i=0;i<20;i++) {
            if (datas->isClose[i] != 0) { 
                pthread_kill(datas->threadToClose[i], SIGTERM); // Closed the requested thread
                datas->isClose[i] = 0;
            }
        }
    }
    pthread_exit(0);
}

int actualIndex(data* data) { // Returns the index of a cell corresponding to a socket id
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

int nextEmpty(data* data) { // Returns the index of the first empty cell of an array
    for (int i=0;i<20;i++) {
        if (data->arrayId[i] == -1) {
            return i;
        }
    }
    return -1;
}

void deleteUser(data* data, int id) { // Removes a user from the user list
    for (int i=0;i<20;i++) {
        if (data->arrayId[i] == id) {
            data->arrayId[i] = -1;
            strcpy(data->arrayName[i],"empty");
            return;
        }
    }
}

int isCommand(char* msg) { // Determines if the received message contains a command
    if ((msg[0] == '/') || (msg[0] == '&')) {
        return 1;
    }
    else {
        return 0;
    }
}

char** getCommand(char* msg) { // Separates the information in a message
    char * save = (char*)malloc(sizeof(char)*strlen(msg));
    strcpy(save,msg);
    printf("%s\n",msg);
    char *datas[3];
    for (int i=0;i<3;i++) {
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

    char* str = (char*)malloc(sizeof(char)*strlen(msg));

    int iterator1 = 0;
    int iterator2 = 0;
    int iterator3 = 0; 
    while(save[iterator1] != '\0') { // Retrieve the message
        if (iterator2 >= 2) {
            str[iterator3] = save[iterator1];
            iterator3++;
        }
        if ((save[iterator1] == ' ') && (iterator2 <= 1)) {
            iterator2++;
        }
    iterator1++;
    }

    strcpy(datas[2],str);
    free(save);

    return datas;
}

void executeCommand(char* content, data* data, int id) {
    char ** command = getCommand(content);
    char * toCompare = command[0];
    strtok(toCompare,"\0");
    strtok(toCompare,"\n");
    if (strcmp(toCompare,"/help") == 0) { // Help command
        char cont[500] = "";
        personalMessage(helpMessage(&cont), data->arrayName[id], data,id);
    }
    else if (strcmp(toCompare,"/files") == 0) { // Files list of the server
        char cont[500] = "";
        personalMessage(listFile(&cont), data->arrayName[id], data,id);
    }
    else if (strcmp(toCompare,"/msg") == 0) { // Private message
        privateMessage(command[2], command[1], data,id);
    }
    else if (strcmp(toCompare,"/list") == 0) { // List command
        char cont[500] = "";
        personalMessage(listClient(&cont,data), data->arrayName[id], data,id);
    }
    else if (strcmp(toCompare,"&up") == 0) {
        char * filename = command[1];
        pthread_t threadFile;
        pthread_create(&threadFile, NULL, (void*)file, filename); // Creates a thread that manages the reciving of messages
    }
    else if (strcmp(toCompare,"&dl") == 0) {
        char * filename = command[1];
        pthread_t threadRFile;
        pthread_create(&threadRFile, NULL, (void*)downloadFile, filename); // Creates a thread that manages the reciving of messages
    }
    else {
        printf("Command not found\n");
        char cont[500] = "";
        personalMessage(helpMessage(&cont), data->arrayName[id], data,id);
    }
    free(toCompare);
}

char* listFile(char* content) { // Print files list of the server
    struct dirent *dir;
    struct stat st = {0};

    if (stat("/servFile", &st) == -1) {
        mkdir("/servFile", 0700);
    }

    DIR *d = opendir("./servFile"); 
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            strcat(content,dir->d_name);
            strcat(content,"\n");
        }
        closedir(d);
    }
    return content;
}

int nameToId(char* username, data* data) { // Retrieves the username corresponding to an id
    for (int i=0;i<20;i++) {
        char * var = data->arrayName[i];
        if (strcmp(var,username) == 0) {
            return data->arrayId[i];
        }
        free(var);
    }
    return -1;
}

char* idToName(int id, data* data) { // Retrieves the correct id for a username
    for (int i=0;i<20;i++) {
        int var = data->arrayId[i];
        if (var == id) {
            return data->arrayName[i];
        }
    }
    return -1;
}

void privateMessage(char* msg, char* username, data* data, int index) { // Sends a private message to another user
    char *sender = calloc(30, sizeof(char));
    strcpy(sender,idToName(data->arrayId[index], data));
    int size = strlen(msg) + 16 + strlen(sender);
    char* content = (char *)malloc(sizeof(char)*size);
    strcpy(content,"Whisper from ");
    strcat(content, sender);
    strcat(content, " : ");
    strcat(content, msg);
    int id = nameToId(username, data);
    if (id != -1) {
        int size = 200;
        if (send(id, &size, sizeof(int), 0) == -1) { perror("Error send"); shutdown(id, 2); shutdown(datas.dS,2); exit(0); }
        if (send(id, content, size*sizeof(char),0) == -1) { perror("Error send"); shutdown(id, 2); shutdown(datas.dS,2); exit(0); }
    }
    else {
        printf("error\n");
    }
    free(sender);
    free(content);
}

void personalMessage(char* msg, char* username, data* data, int index) { // Sends a message to the user who executed the command
    int size = strlen(msg);
    char* content = (char *)malloc(sizeof(char)*size);
    strcat(content, msg);
    int id = nameToId(username, data);
    if (id != -1) {
        int size = 500;
        if (send(id, &size, sizeof(int), 0) == -1) { perror("Error send"); shutdown(id, 2); shutdown(datas.dS,2); exit(0); }
        if (send(id, content, size*sizeof(char),0) == -1) { perror("Error send"); shutdown(id, 2); shutdown(datas.dS,2); exit(0); }
    }
    else {
        printf("error\n");
    }
    free(content);
}


char* helpMessage(char* content) {// Reads the list of available commands in a file
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

char * listClient(char* content, data* data) { // Returns the list of online clients
    int iterator1 = 0;
    for (int i=0;i<20;i++) {
        if (data->arrayId[i] != (int*)-1) {
            int iterator2 = 0;
            while((data->arrayName[i])[iterator2] != '\0') {
                content[iterator1] = (data->arrayName[i])[iterator2];
                iterator1++;
                iterator2++;
            }
            content[iterator1] = ' ';
            iterator1++;
        }
    }
    return content;
}

int checkPseudo(data* data, char* pseudo) { // See if a username is already in use
    for (int i=0;i<20;i++) {
        if (strcmp(data->arrayName[i],pseudo) == 0) {
            return 1;
        }
    }
    return 0;
}

void write_file(int sockfd, char* filename){
  int n;
  FILE *fp;
  char buffer[SIZE];
 char * path = malloc(50*sizeof(char));
 strcat(path,"./servFile/");
 strcat(path,filename);
  fp = fopen(path, "w");
  while (1) {
    n = recv(sockfd, buffer, SIZE, 0);
    if (n <= 0){
      break;
      return;
    }
	fputs(buffer,fp);
    fprintf(fp, "%s", buffer);
    bzero(buffer, SIZE);
  }
  	fclose(fp);
  return;
}
 
void file(char* filename){
  char *ip = "127.0.0.1";
  int port = 3030;
  int e;
  char * name = malloc(sizeof(char)*50);

  strcpy(name,filename);
 
  int sockfd, new_sock;
  struct sockaddr_in server_addr, new_addr;
  socklen_t addr_size;
  char buffer[SIZE];
 
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0) {
    perror("[-]Error in socket");
    exit(1);
  }
  printf("[+]Server socket created successfully.\n");
 
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = port;
  server_addr.sin_addr.s_addr = inet_addr(ip);
 
  e = bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
  if(e < 0) {
    perror("[-]Error in bind");
    exit(1);
  }
  printf("[+]Binding successfull.\n");
 
  if(listen(sockfd, 10) == 0){
 printf("[+]Listening....\n");
 }else{
 perror("[-]Error in listening");
    exit(1);
 }

 strtok(name,"\n");
 
  addr_size = sizeof(new_addr);
  new_sock = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size);
  write_file(new_sock,name);
  printf("[+]Data written in the file successfully.\n");

  pthread_exit(0);
 
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
 
void downloadFile(char* filename){
    char *ip = "127.0.0.1";
  int port = 3033;
  int e;
  char * name = malloc(50*sizeof(char));
    strcat(name,"./servFile/");

  strcat(name,filename);

  printf("%s\n",name);

  strtok(name,"\n");
 
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
  server_addr.sin_addr.s_addr = inet_addr(ip);
 
  e = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));

  while (e==-1) { 
    e = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
  }
 //printf("[+]Connected to Server.\n");
 
  fp = fopen(name, "r");
  if (fp == NULL) {
    perror("[-]Error in reading file.");
    exit(1);
  }
 
  send_file(fp, sockfd);
  //printf("[+]File data sent successfully.\n");
  printf("File send.\n");
 
  //printf("[+]Closing the connection.\n");
  close(sockfd);
  //shutdown(sockfd, 2); // #
 
  pthread_exit(0);

}
