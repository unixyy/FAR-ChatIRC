
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <pthread.h>
#include <dirent.h>
#include "funcServ.h"
#include "threadServ.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <regex.h>

#define SIZE 1024
#define haveToStop "@quit"

/**
 * @brief Thread for receiving and sending a message
 * 
 * @param datas Information used for server management
 */
void * receiveSend(data* datas) { 
    int index = actualIndex(datas); // Index of the current client in the client array
    int stop = 0;
    int firstmsg = 1;

    personalMessage("\033[37;1;7mWelcome! You can chat freely after entering your nickname (/help if needed).\033[0m", datas->arrayName[index], datas,index);  // Send a welcome message to the client
   
    while (!stop && !datas->close && !datas->isClose[index]) {
        int size;
        int sizeRCV= recv((int)(size_t)datas->arrayId[index], &size, sizeof(int), 0); // Receives the size of the message that will follow
        if (sizeRCV== -1) { break; }
        else if (sizeRCV== 0) { break; }

        char *msg = (char *)malloc(sizeof(char)*size);
        int msgRCV = recv((int)(size_t)datas->arrayId[index], msg, size*sizeof(char), 0); // Receives a message from a client
        if (msgRCV == -1 ) { break; }
        else if (msgRCV == 0) { break; }

        if(firstmsg == 1) { // First message sent (pseudo of the client)
            if (checkPseudo(datas,msg) == 1) { // Nickname invalid
                personalMessage("\033[32;1;1m## Nickname invalid, please choose another one ##\033[0m", datas->arrayName[index], datas,index); // Send a new nickname request
            } 
            else {
                pthread_mutex_lock(&datas->mutex);
                strtok(msg,"\n");
                strcpy(datas->arrayName[index],msg);
                pthread_mutex_unlock(&datas->mutex);

                personalMessage("\033[32;1;1m## Nickname registered ##\033[0m", datas->arrayName[index], datas,index); // Confirm that the nickname is correct
                firstmsg=0;
            }
        }
        else if (isCommand(msg)) { // Command in the message
            executeCommand(msg,datas,index);
        }
        else {
            strtok(haveToStop, "\0");
            if (strcmp(msg, haveToStop) == 0) {  stop = 1; } // If the message sent is @quit
            else { messageChannel(datas,msg,index);} // Send a message in the channel
        }
        free(msg);
  }
  shutdown((int)(size_t)datas->arrayId[index],2); // Close the connection

  // Manages the deletion of the client
  pthread_mutex_lock(&datas->mutex);
  deleteUser(datas,(int)(size_t)datas->arrayId[index]);
  datas->threadToClose[index] = (void*)pthread_self();
  datas->isClose[index] = (int*)(size_t)1;
  datas->arrayIdChannel[index] = NULL;
  pthread_mutex_unlock(&datas->mutex);

  pthread_exit(0);
}

/**
 * @brief Thread for closing a receiveSend thread 
 * 
 * @param datas Information used for server management
 */
void * closeThread(data* datas) {
    while(!datas->close) { 
        sleep(10);
        for (int i=0;i<20;i++) {
            if (datas->isClose[i] != 0) { 
                pthread_join(*datas->threadToClose[i], NULL); // Closed the requested thread
                pthread_mutex_lock(&datas->mutex);
                datas->isClose[i] = 0;
                pthread_mutex_unlock(&datas->mutex);
                rk_sema_post(datas->s);
            }
        }
    }
    for (int k=0;k<20;k++) {
        personalMessage("\033[32;1;1m## Server is closing ##\033[0m", datas->arrayName[k], datas,k); // Send a message to all the clients
    }
    for (int j=0;j<24;j++) {
        if ((j!=20) && (datas->threadToClose[j]!=NULL)) {
            pthread_join(*datas->threadToClose[j], NULL); // Closed the requested thread
        }
    }
    pthread_exit(0);
}

/**
 * @brief Receiving a file
 * 
 */
void file(data* datas) {
    struct stat st = {0};
    if (stat("./servFile", &st) == -1) { mkdir("./servFile", 0700);}

    char *ip = "127.0.0.1";
    int port = 3030;
    int e;
 
    int sockfd, new_sock;
    struct sockaddr_in server_addr, new_addr;
    socklen_t addr_size;
    char buffer[SIZE];
 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    if(sockfd < 0) { perror("[-]Error in socket"); exit(1); }
 
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    
    e = bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(e < 0) { perror("[-]Error in bind"); exit(1); }
    
    if(listen(sockfd, 10) != 0) { perror("[-]Error in listening"); exit(1); }

    struct rk_sema s;
    rk_sema_init(&s, 1);

    while (!datas->close) {
        rk_sema_wait(&s);

        addr_size = sizeof(new_addr);
        new_sock = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size); // Accept a client
        if (new_sock < 0) { perror("[-]Accept failed"); exit(1); }

        while (recv(new_sock,buffer,50,0) > 0) {
            strtok(buffer,"\n");
            write_file(new_sock,buffer); // Receive the file
        }

        rk_sema_post(&s);
        close(new_sock);
    }
    datas->threadToClose[23] = (void*)pthread_self();
    pthread_exit(0);
}

/**
 * @brief Sending a file
 * 
 */
void downloadFile(data* datas) { 
    struct stat st = {0};
    if (stat("./servFile", &st) == -1) { mkdir("./servFile", 0700);}

    char *ip = "127.0.0.1";
    int port = 3033;
    int e;
 
    FILE *fp;
    int sockfd, new_sock;
    struct sockaddr_in server_addr, new_addr;
    socklen_t addr_size;
    char buffer[SIZE];
 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    if(sockfd < 0) { perror("[-]Error in socket"); exit(1); }
 
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);
 
    e = bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(e < 0) { perror("[-]Error in bind"); exit(1); }
 
    if(listen(sockfd, 10) != 0) { perror("[-]Error in listening"); exit(1); }

    struct rk_sema s;
    rk_sema_init(&s, 1);

    while (!datas->close) {
        rk_sema_wait(&s);    

        addr_size = sizeof(new_addr);
        new_sock = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size); // Accept a client
        if (new_sock < 0) { perror("[-]Accept failed"); exit(1); }

        while (recv(new_sock,buffer,50,0) > 0) {
            char* test = malloc(50*sizeof(char));
            strtok(buffer,"\n");

            strcat(test,"./servFile/");
            strcat(test,buffer);

            struct dirent *dir;
            int stop = 0;
            DIR *d = opendir("./servFile"); 
            if (d) {
                while ((dir = readdir(d)) != NULL) { // There are some files
                    if (strcmp(buffer,dir->d_name)==0) { stop = 1; }
                }
                closedir(d);
            }

            if (stop) { // Filename exists
                fp = fopen(test, "r");
                if (fp == NULL) { perror("[-]Error in reading file."); exit(1);}
                send_file(fp, new_sock); // Send the file
            } else {
                char data[SIZE] = {0};
                send(new_sock, data, sizeof(data), 0);
            }       

            free(test);
        }
        rk_sema_post(&s);
        close(new_sock);
    }
    datas->threadToClose[22] = (void*)pthread_self();
    pthread_exit(0);
}

/**
 * @brief Admin session
 * 
 * @param data Information used for server management
 */
void admin(data* data) {
    // Informative display
    printf("\033[31;1;1mAdmin connection...\n\n\033[0m");
    printf("\033[37;1;7mIf you want to shut down the server you must write @quit and press Ctrl-C !\n\n\033[0m");
    printf("\033[37;1;7mCommand list :\n\033[0m");
    printf("\033[37;1;7m- /kick user\n\033[0m");
    printf("\033[37;1;7m- /msg user content\n\033[0m");
    printf("\033[37;1;7m- /all content\n\n\033[0m");

    while(!data->close) { 
        char *m = (char *)malloc(sizeof(char)*100);
        fgets(m, sizeof(char)*100, stdin);
        printf("\n");
        strtok(m,"\n");

        if (strcmp(m,haveToStop)==0) { printf("\033[31;1;%dmAdmin disconnection...\n\033[0m",1); break; } // Quit the admin session
        else if (isCommand(m)) { adminCommand(m, data); } // Special command
        else { printf("\033[32;1;1m## Command not found ##\n\n\033[0m"); }
    }
    data->threadToClose[21] = (void*)pthread_self();
    pthread_exit(0);
}
