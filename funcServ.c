
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
            strtok(haveToStop, "\0");
            if (strcmp(msg, haveToStop) == 0) { // If the message sent is @quit
                    stop = 1; 
                    } 
            else {
            messageChannel(datas,msg,index);
            }
        }
        free(msg);
  }
  shutdown(datas->arrayId[index],2); // Close the client
  pthread_mutex_lock(&mutex);
  deleteUser(datas,datas->arrayId[index]);
  datas->threadToClose[index] = pthread_self();
  datas->isClose[index] = 1;
  datas->arrayIdChannel[index] = NULL;
  pthread_mutex_unlock(&mutex);
  rk_sema_post(datas->s);
  pthread_exit(0);
}

void broadcast(data* datas, char* msg, int index) { // Broadcast a message to all the clients
    char *sender = calloc(30, sizeof(char));
    strcpy(sender,idToName(datas->arrayId[index], datas));
    strtok(sender, "\n");
    int size = 103 + strlen(sender); 
    char* content = (char *)malloc(sizeof(char)*size);
    strcpy(content,"");
    strcat(content,sender);
    strcat(content, " : ");
    strcat(content, msg);
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

void messageChannel(data* datas, char* msg, int index) { 
    char *sender = calloc(30, sizeof(char));
    strcpy(sender,idToName(datas->arrayId[index], datas));
    strtok(sender, "\n");
    int size = strlen(msg) + 3 + strlen(sender); 
    char* content = (char *)malloc(sizeof(char)*size);
    strcpy(content,"");
    strcat(content, sender);
    strcat(content, " : ");
    strcat(content, msg);
    strtok(msg, "\n");
    int myChannel = datas->arrayIdChannel[index];
        for (int i=0;i<20;i++) {
            if ((datas->arrayId[index] != datas->arrayId[i]) && (datas->arrayId[i] != -1) && ((datas->arrayIdChannel[i])==myChannel)) { // If the client is different from the one sending
                if (send(datas->arrayId[i], &size, sizeof(int), 0) == -1) { perror("Error send"); shutdown(datas->arrayId[index], 2); shutdown((*datas).dS,2); exit(0); }
            }
        }
        for (int i=0;i<20;i++) {
            if ((datas->arrayId[index] != datas->arrayId[i]) && (datas->arrayId[i] != -1) && ((datas->arrayIdChannel[i])==myChannel)) { // If the client is different from the one sending
                if (send(datas->arrayId[i], content, size*sizeof(char), 0) == -1) {  perror("Error send"); shutdown(datas->arrayId[index], 2); shutdown((*datas).dS, 2); exit(0); }
                    printf("Message sent\n");
            }
        }
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
        if (data->arrayId[cpt] == data->actualId) { stop = 1; }
        cpt++;
    }
    return cpt-1;
}

int nextEmpty(data* data) { // Returns the index of the first empty cell of an array
    for (int i=0;i<20;i++) {
        if (data->arrayId[i] == -1) { return i; }
    }
    return -1;
}

int nextEmptyChannel(data* data) { // Returns the index of the first empty cell of an array
    for (int i=0;i<20;i++) {
        if (strcmp(data->arrayChannelName[i],"empty")==0) { return i; }
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
    if ((msg[0] == '/') || (msg[0] == '&')) { return 1; }
    else { return 0; }
}

char** getCommand(char* msg) { // Separates the information in a message
    char * save = (char*)malloc(sizeof(char)*strlen(msg));
    strcpy(save,msg);
    char *datas[3];
    for (int i=0;i<3;i++) { datas[i]=(char *)malloc(sizeof(char) * 200); }

    char d[] = " ";
    char *p = strtok(msg, d);
    int i = 0;
    while((p != NULL) && (i<2)) { // Recover the first two parts
        strcpy(datas[i],p);
        p = strtok(NULL, d);
        i++;
    }

    char* str = (char*)malloc(sizeof(char)*strlen(msg));

    int iterator1 = 0;
    int iterator2 = 0;
    int iterator3 = 0; 
    while(save[iterator1] != '\0') { // Retrieve the message
        if (iterator2 >= 2) { str[iterator3] = save[iterator1]; iterator3++; }
        if ((save[iterator1] == ' ') && (iterator2 <= 1)) { iterator2++; }
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
        char cont[1000] = "";
        personalMessage(helpMessage(&cont), data->arrayName[id], data,id);
    } else if (strcmp(toCompare,"/files") == 0) { // Files list of the server
        char cont[500] = "";
        personalMessage(listFile(&cont), data->arrayName[id], data,id);
    } else if (strcmp(toCompare,"/msg") == 0) { // Private message
        privateMessage(command[2], command[1], data,id);
    } else if (strcmp(toCompare,"/list") == 0) { // List command
        char cont[500] = "";
        personalMessage(listClient(&cont,data), data->arrayName[id], data,id);
    } else if (strcmp(toCompare,"/listC") == 0) { // List command
        char cont[500] = "";
        personalMessage(listClientChannel(&cont,data,id), data->arrayName[id], data,id);
    } 
    else if (strcmp(toCompare,"/connect") == 0) {
        connectChannel(command[1], id, data);
    }
    else if (strcmp(toCompare, "/create") == 0)
    {
        createChannel(command[1],data);
    }
    else if (strcmp(toCompare, "/delete") == 0)
    {
        deleteChannel(command[1],data);
    }
    else if (strcmp(toCompare, "/all") == 0)
    {
        broadcast(data, command[2], id);
    }
    else if (strcmp(toCompare, "/report") == 0)
    {
        // ! TODO : CA FAIT BIZARRE
        printf("ok\n");
        printf("%s\n",data->arrayName[id]);
        printf("%s\n",command[1]);
        printf("%s\n",command[2]);
        printf("%s : %s %s\n",data->arrayName[id],command[1],command[2]);
    }
    else if (strcmp(toCompare, "/chann") == 0)
    {
        char cont[500] = "";
        personalMessage(listChannels(&cont,data), data->arrayName[id], data,id);
    }
    else
    {
        printf("Command not found\n");
        char cont[500] = "";
        personalMessage(helpMessage(&cont), data->arrayName[id], data,id);
    }
    free(toCompare);
}

char* listFile(char* content) { // Print files list of the server
    struct dirent *dir;
    DIR *d = opendir("./servFile"); 
    if (d) {
        while ((dir = readdir(d)) != NULL) { strcat(content,dir->d_name); strcat(content,"\n"); } // There are some files
        closedir(d);
    }
    return content;
}

int nameToId(char* username, data* data) { // Retrieves the username corresponding to an id
    for (int i=0;i<20;i++) {
        if (strcmp(data->arrayName[i],username) == 0) { return data->arrayId[i]; }
    }
    return -1;
}

char* idToName(int id, data* data) { // Retrieves the correct id for a username
    for (int i=0;i<20;i++) {
        int var = data->arrayId[i];
        if (var == id) { return data->arrayName[i]; }
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
    if (id != -1) { // Correct username
        int size = 200;
        if (send(id, &size, sizeof(int), 0) == -1) { perror("Error send"); shutdown(id, 2); shutdown(datas.dS,2); exit(0); }
        if (send(id, content, size*sizeof(char),0) == -1) { perror("Error send"); shutdown(id, 2); shutdown(datas.dS,2); exit(0); }
    } else { printf("error\n"); }

    free(sender);
    free(content);
}

void personalMessage(char* msg, char* username, data* data, int index) { // Sends a message to the user who executed the command
    int size = strlen(msg);
    char* content = (char *)malloc(sizeof(char)*size);
    strcat(content, msg);
    int id = nameToId(username, data);

    if (id != -1) { // Correct username
        int size = 1000;
        if (send(id, &size, sizeof(int), 0) == -1) { perror("Error send"); shutdown(id, 2); shutdown(datas.dS,2); exit(0); }
        if (send(id, content, size*sizeof(char),0) == -1) { perror("Error send"); shutdown(id, 2); shutdown(datas.dS,2); exit(0); }
    } else { printf("error\n");}
    free(content);
}

char* helpMessage(char* content) {// Reads the list of available commands in a file
    FILE *f;
    char c;
    f = fopen("listCommand.txt", "rt");
    int i = 0;
    while((c=fgetc(f))!=EOF){ content[i] = c; i++; } // There are some lines
    fclose(f);
    return content;
}

void channelList(data* data){
    char* content = malloc(1000*sizeof(char));
    FILE *f;
    char c;
    f = fopen("listChannel.txt", "rt");
    int i = 0;
    while((c=fgetc(f))!=EOF){ 
        content[i] = c;
        i++; 
    } // There are some lines
    fclose(f);

    char d[] = "\n";
    char *p = strtok(content, d);
    int j = 0;
    while((p != NULL) && (j<20)) { // Recover the first two parts
        strcpy(data->arrayChannelName[j],p);
        p = strtok(NULL, d);
        j++;
    }

}

void saveChannels(data* data) {
    FILE *f;
    char c;
    f = fopen("listChannel.txt", "w");
    int i = 0;
    while(i<20){ 
        if (strcmp(data->arrayChannelName[i],"empty")!=0) {
            fwrite(data->arrayChannelName[i], 1, strlen(data->arrayChannelName[i]), f);   
            fwrite("\n", 1, 1, f);   
        }
        i++; 
    }
    fclose(f);
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

char * listClientChannel(char* content, data* data, int id) { // Returns the list of online clients
    int iterator1 = 0;
    for (int i=0;i<20;i++) {
        if ((data->arrayIdChannel[i] == data->arrayIdChannel[id]) && (strcmp(data->arrayName[i],"empty") != 0)) {
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
        if (strcmp(data->arrayName[i],pseudo) == 0) { return 1; }
    }
    return 0;
}

int checkChannel(data* data, char* channel) { // See if a username is already in use
    for (int i=0;i<20;i++) {
        if (strcmp(data->arrayChannelName[i],channel) == 0) { return 1; }
    }
    return 0;
}

void write_file(int sockfd, char* filename) { // Create a new file
    int n;
    FILE *fp;
    char buffer[SIZE];
    char * path = malloc(50*sizeof(char));

    strcat(path,"./servFile/");
    strcat(path,filename);
    
    fp = fopen(path, "w");
    while (1) { // Some content
        n = recv(sockfd, buffer, SIZE, 0);
        if (n <= 0){ break; return;}
        fputs(buffer,fp);
        bzero(buffer, SIZE);
    }
    fclose(fp);
    return;
}
 
void file() { // Receiving a file
    printf("Receiving file thread : \n");

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
    printf("[+]Server socket created successfully.\n");
 
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    
    e = bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(e < 0) { perror("[-]Error in bind"); exit(1); }
    printf("[+]Binding successfull.\n");
    
    if(listen(sockfd, 10) == 0) { printf("[+]Listening....\n");}
    else { perror("[-]Error in listening"); exit(1); }

    struct rk_sema s;
    rk_sema_init(&s, 1);

    while (1) {
        rk_sema_wait(&s);

        addr_size = sizeof(new_addr);
        new_sock = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size); // Accept a client
        if (new_sock < 0) { perror("accept failed"); exit(1); }
	    puts("Connection accepted");

        while (recv(new_sock,buffer,50,0) > 0) {
            strtok(buffer,"\n");
            write_file(new_sock,buffer); // Receive the file
            printf("[+]Data written in the file successfully.\n");
        }

    rk_sema_post(&s);
    close(new_sock);
    }
    pthread_exit(0);
}

void send_file(FILE *fp, int sockfd) { // Send the file
    int n;
    char data[SIZE] = {0};
    while(fread(data, sizeof(char), SIZE, fp)!= NULL) { // There are some data
        if (send(sockfd, data, sizeof(data), 0) == -1) { perror("[-]Error in sending file."); exit(1);}
        bzero(data, SIZE);
    }
}
 
void downloadFile() { // Sending a file
    printf("Sending file thread : \n");

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
    printf("[+]Server socket created successfully.\n");
 
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);
 
    e = bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(e < 0) { perror("[-]Error in bind"); exit(1); }
    printf("[+]Binding successfull.\n");
 
    if(listen(sockfd, 10) == 0) { printf("[+]Listening....\n"); }
    else { perror("[-]Error in listening"); exit(1); }

    struct rk_sema s;
    rk_sema_init(&s, 1);

    while (1) {
        rk_sema_wait(&s);    

        addr_size = sizeof(new_addr);
        new_sock = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size); // Accept a client
        if (new_sock < 0) { perror("accept failed"); exit(1); }
        puts("Connection accepted");

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
                printf("File send.\n");
            } else {
                char data[SIZE] = {0};
                send(new_sock, data, sizeof(data), 0);
                printf("This file does not exist.\n");
            }       
            free(test);
        }
    rk_sema_post(&s);
    close(new_sock);
    }
  pthread_exit(0);
}


void createChannel(char* channel, data* data){
    int id = nextEmptyChannel(data);
    if (id != -1) { 
        if (checkChannel(data, channel) == 0) {
            strcpy(datas.arrayChannelName[id],channel);
        }
    }   
}

void deleteChannel(char* channel, data* data){
    for (int i=0; i<20; i++) {
        if ((strcmp(data->arrayChannelName[i],channel) == 0) && (strcmp(channel,"public") != 0)) { 
            for (int j=0; j<20; j++) {
                if (data->arrayIdChannel[j]==i) {
                    data->arrayIdChannel[j] = 0;
                }
            }
            strcpy(data->arrayChannelName[i],"empty");
        }
    }
}

int nameToIdChannel(char* channel, data* data) { // Retrieves the username corresponding to an id
    for (int i=0;i<20;i++) {
        if (strcmp(data->arrayChannelName[i],channel) == 0) { return i; }
    }
    return -1;
}

void connectChannel(char* channel, int index, data* data){
    if (checkChannel(data, channel) == 1) {
        strtok(channel,"\n");
        data->arrayIdChannel[index] = nameToIdChannel(channel,data);
    }
}

char* listChannels(char* content, data* data){
    int iterator1 = 0;
    for (int i=0;i<20;i++) {
        if (strcmp(data->arrayChannelName[i],"empty")!=0) {
            int iterator2 = 0;
            while((data->arrayChannelName[i])[iterator2] != '\0') {
                content[iterator1] = (data->arrayChannelName[i])[iterator2];
                iterator1++;
                iterator2++;
            }
            content[iterator1] = ' ';
            iterator1++;
        }
    }
    return content;
}
