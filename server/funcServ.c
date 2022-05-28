
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
 * @brief Send a message to all the clients
 * 
 * @param datas Information used for server management
 * @param text1 First part of the message to send
 * @param text2 Second part of the message to send
 * @param index Index of the client 
 */
void broadcast(data* datas, char* text1,char* text2, int index) { 
    char *sender = calloc(40, sizeof(char));
    strcpy(sender,idToName((int)(size_t)datas->arrayId[index], datas));
    strtok(sender, "\n");
    int size = 200;
    char* content = (char *)malloc(sizeof(char)*size);

    // Message to send
    strcpy(content,"[ALL] ");
    strcat(content,sender);
    strcat(content, " : ");
    strcat(content, text1);
    strcat(content, " ");
    strcat(content, text2);

    // Sending management
    for (int i=0;i<20;i++) {
        if ((datas->arrayId[index] != datas->arrayId[i]) && ((int)(size_t)datas->arrayId[i] != -1)) { // If the client is different from the one sending
            personalMessage(content, datas->arrayName[index], datas, index);
        }
    }
}

/**
 * @brief Sending a message to all clients by an admin
 * 
 * @param datas Information used for server management
 * @param text1 First part of the message to send
 * @param text2 Second part of the message to send
 */
void adminBroadcast(data* datas, char* text1,char* text2) {
    int size = 200;
    char* content = (char *)malloc(sizeof(char)*size);

    // Message to send
    strcpy(content,"[ALL] admin : ");
    strcat(content, text1);
    strcat(content, " ");
    strcat(content, text2);

    // Sending management
    for (int i=0;i<20;i++) {
        if ((int)(size_t)datas->arrayId[i] != -1) { // If the client exists
            if (send((int)(size_t)datas->arrayId[i], &size, sizeof(int), 0) == -1) { perror("[-]Error send"); shutdown((*datas).dS,2); exit(0); }
        }
    }
    for (int i=0;i<20;i++) {
        if ((int)(size_t)datas->arrayId[i] != -1) { // If the client exists
            if (send((int)(size_t)datas->arrayId[i], content, size*sizeof(char), 0) == -1) {  perror("[-]Error send"); shutdown((*datas).dS, 2); exit(0); }
        }
    }
}

/**
 * @brief Send a message in the current channel
 * 
 * @param datas Information used for server management
 * @param msg Message to send
 * @param index Index of the client 
 */
void messageChannel(data* datas, char* msg, int index) { 
    int size = strlen(msg) + 50;
    char* content = (char *)malloc(sizeof(char)*size);
    int myChannel = (int)(size_t)datas->arrayIdChannel[index];

    // Message to send
    strcpy(content,"");
    strcat(content, idToName((int)(size_t)datas->arrayId[index], datas));
    strcat(content, " : ");
    strcat(content, msg);

    // Sending management
    for (int i=0;i<20;i++) {
        if ((datas->arrayId[index] != datas->arrayId[i]) && ((int)(size_t)datas->arrayId[i] != -1) && ((int)(size_t)(datas->arrayIdChannel[i])==myChannel)) { // If the client is different from the one sending
            personalMessage(content, datas->arrayName[index], datas, index);
        }
    }
}

/**
 * @brief Calculate the index of a cell corresponding to a socket id
 * 
 * @param data Information used for server management
 * @return The index, -1 if not found
 */
int actualIndex(data* data) {
    int cpt = 0;
    int stop = 0;
    while (!stop) { 
        if ((int)(size_t)data->arrayId[cpt] == data->actualId) { stop = 1; }
        cpt++;
    }
    return cpt-1;
}

/**
 * @brief Calculate the index of the first empty cell of the name array 
 * 
 * @param data Information used for server management
 * @return The index, -1 if not found 
 */
int nextEmpty(data* data) { 
    for (int i=0;i<20;i++) {
        if (data->arrayId[i] == (int*)(size_t)-1) { return i; }
    }
    return -1;
}

/**
 * @brief Calculate the index of the first empty cell of the channel array
 * 
 * @param data Information used for server management
 * @return The index, -1 if not found 
 */
int nextEmptyChannel(data* data) {
    for (int i=0;i<20;i++) {
        if (strcmp(data->arrayChannelName[i],"empty")==0) { return i; }
    }
    return -1;
}

/**
 * @brief Removes a client from the client list
 * 
 * @param data Information used for server management
 * @param id The id of the client
 */
void deleteUser(data* data, int id) {
    for (int i=0;i<20;i++) {
        if (data->arrayId[i] == (int*)(size_t)id) {
            data->arrayId[i] = (int*)(size_t)-1;
            strcpy(data->arrayName[i],"empty");
            return;
        }
    }
}

/**
 * @brief Determines if the received message contains a command
 * 
 * @param msg The message to check
 * @return 1 if this is command, O otherwise
 */
int isCommand(char* msg) { 
    if ((msg[0] == '/') || (msg[0] == '&')) { return 1; }
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
    static char *datas[3];
    for (int i=0;i<3;i++) { datas[i]=(char *)malloc(sizeof(char) * 200); } // Initialize the returned array

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

/**
 * @brief Executes a particular command
 * 
 * @param content The message to check
 * @param data Information used for server management
 * @param id The id of the client
 */
void executeCommand(char* content, data* data, int id) {
    char ** command = getCommand(content);
    char * toCompare = command[0];
    strtok(toCompare,"\0");
    strtok(toCompare,"\n");

    if (strcmp(toCompare,"/help") == 0) { char cont[1000] = ""; personalMessage(helpMessage((char*)&cont), data->arrayName[id], data,id); } // Help command 
    else if (strcmp(toCompare,"/files") == 0) { char cont[500] = ""; personalMessage(listFile((char*)&cont), data->arrayName[id], data,id); } // Files list of the server 
    else if (strcmp(toCompare,"/msg") == 0) { privateMessage(command[2], command[1], data,id); } // Private message 
    else if (strcmp(toCompare,"/list") == 0) { char cont[500] = ""; personalMessage(listClient((char*)&cont,data), data->arrayName[id], data,id); } // List online clients 
    else if (strcmp(toCompare,"/listC") == 0) { char cont[500] = ""; personalMessage(listClientChannel((char*)&cont,data,id), data->arrayName[id], data,id); } // List online clients in the current channel
    else if (strcmp(toCompare,"/connect") == 0) { pthread_mutex_lock(&data->mutex); connectChannel(command[1],id,data); pthread_mutex_unlock(&data->mutex); } // Connect to a channel
    else if (strcmp(toCompare, "/create") == 0) { pthread_mutex_lock(&data->mutex); createChannel(command[1],data,id); pthread_mutex_unlock(&data->mutex); } // Create a channel 
    else if (strcmp(toCompare, "/delete") == 0) { pthread_mutex_lock(&data->mutex); deleteChannel(command[1],data,id); pthread_mutex_unlock(&data->mutex); } // Delete a channel
    else if (strcmp(toCompare, "/all") == 0) { broadcast(data, command[1],command[2], id); } // Broadcast a message to all channels
    else if (strcmp(toCompare, "/report") == 0) { report(data->arrayName[id],command[1],command[2]); } // Report a situation to an admin
    else if (strcmp(toCompare, "/chann") == 0) { char cont[500] = ""; personalMessage(listChannels((char*)&cont,data), data->arrayName[id], data,id); } // List channels
    else { printf("## Command not found ##\n"); char cont[500] = ""; personalMessage(helpMessage((char*)&cont), data->arrayName[id], data,id); }

    free(toCompare);
}

/**
 * @brief Executes a particular admin command 
 * 
 * @param content The content to check
 * @param data Information used for server management
 */
void adminCommand(char* content, data* data) {
    char ** command = getCommand(content);
    char * toCompare = command[0];
    strtok(toCompare,"\0");
    strtok(toCompare,"\n");

    if (strcmp(toCompare,"/msg") == 0) { adminPrivateMessage(command[2], command[1], data); } // Private message 
    else if (strcmp(toCompare, "/all") == 0) { adminBroadcast(data, command[1],command[2]); } // Message to all channels
    else if (strcmp(toCompare, "/kick") == 0) { kick(data, command[1]); } // Kick a client
    else { printf("## Command not found ##\n"); }
    
    free(toCompare);
}

/**
 * @brief Report a situation to an admin
 * 
 * @param id The id of the client
 * @param text1 First part of the message
 * @param text2 Second part of the message
 */
void report(char* id, char* text1, char* text2){
    char cont[500] = "";
    strcpy(cont,id);

    strcat(cont," : ");
    strcat(cont,text1);
    strcat(cont,text2);

    printf("%s\n\n",cont);
}

/**
 * @brief Kick a client from the server
 * 
 * @param data Information used for server management
 * @param name The name of the client
 */
void kick(data* data, char* name) {
    int id = nameToId(name,data);
    if (id!=-1) { // User exists
        adminPrivateMessage("You have been kicked !", name, data);
        printf("## Client kicked ##\n\n");
        shutdown((int)(size_t)data->arrayId[id],2);
        deleteUser(data,id);

        // TO DO :
        /*pthread_mutex_lock(&datas->mutex);
        deleteUser(datas,id);
        datas->threadToClose[index] = (void*)pthread_self();
        datas->isClose[index] = (int*)(size_t)1;
        datas->arrayIdChannel[index] = NULL;
        pthread_mutex_unlock(&datas->mutex);*/
    }
}

/**
 * @brief Print files list of the server
 * 
 * @param content Place to put the list
 * @return The content
 */
char* listFile(char* content) {
    struct dirent *dir;
    DIR *d = opendir("./servFile"); 
    if (d) {
        while ((dir = readdir(d)) != NULL) { strcat(content,dir->d_name); strcat(content,"\n"); } // There are some files
        closedir(d);
    }
    return content;
}

/**
 * @brief Retrieves the username corresponding to an id
 * 
 * @param username The username to transform
 * @param data Information used for server management
 * @return The id of the client, -1 if not found
 */
int nameToId(char* username, data* data) {
    for (int i=0;i<20;i++) {
        if (strcmp(data->arrayName[i],username) == 0) { return (int)(size_t)data->arrayId[i]; }
    }
    return -1;
}

/**
 * @brief Retrieves the correct id for a username
 * 
 * @param id The id to transform
 * @param data Information used for server management
 * @return The name of the client, "" if not found
 */
char* idToName(int id, data* data) { 
    for (int i=0;i<20;i++) {
        int var = (int)(size_t)data->arrayId[i];
        if (var == id) { return data->arrayName[i]; }
    }
    return "";
}

/**
 * @brief Sends a private message to another user
 * 
 * @param msg The message to send
 * @param username The username of the sender
 * @param data Information used for server management
 * @param index The index of the sender
 */
void privateMessage(char* msg, char* username, data* data, int index) { 
    char *sender = calloc(40, sizeof(char));
    strcpy(sender,idToName((int)(size_t)data->arrayId[index], data));
    int size = strlen(msg) + 50;
    char* content = (char *)malloc(sizeof(char)*size);

    strcpy(content,"Whisper from ");
    strcat(content, sender);
    strcat(content, " : ");
    strcat(content, msg);

    int id = nameToId(username, data);
    if (id != -1) { // Correct username
        int size = 200;
        if (send(id, &size, sizeof(int), 0) == -1) { perror("[-]Error send"); shutdown(id, 2); shutdown(datas.dS,2); exit(0); }
        if (send(id, content, size*sizeof(char),0) == -1) { perror("[-]Error send"); shutdown(id, 2); shutdown(datas.dS,2); exit(0); }
    } 

    free(sender);
    free(content);
}

/**
 * @brief Sends a private message to another user by an admin
 * 
 * @param msg The message to send
 * @param username The username of the receiver
 * @param data Information used for server management
 */
void adminPrivateMessage(char* msg, char* username, data* data) { 
    int size = strlen(msg) + 20;
    char* content = (char *)malloc(sizeof(char)*size);

    strcpy(content,"Whisper from admin : ");
    strcat(content, msg);

    int id = nameToId(username, data);
    if (id != -1) { // Correct username
        int size = 200;
        if (send(id, &size, sizeof(int), 0) == -1) { perror("[-]Error send");shutdown(datas.dS,2); exit(0); }
        if (send(id, content, size*sizeof(char),0) == -1) { perror("[-]Error send");shutdown(datas.dS,2); exit(0); }
    }

    free(content);
}

/**
 * @brief // Sends a message to the user who executed the command
 * 
 * @param msg The message to send
 * @param username The username of the sender
 * @param data Information used for server management
 * @param index The index of the client
 */
void personalMessage(char* msg, char* username, data* data, int index) { 
    int size = strlen(msg);
    char* content = (char *)malloc(sizeof(char)*size);
    strcat(content, msg);

    int id = nameToId(username, data);
    if (id != -1) { // Correct username
        int size = 1000;
        if (send(id, &size, sizeof(int), 0) == -1) { perror("[-]Error send"); shutdown(id, 2); shutdown(datas.dS,2); exit(0); }
        if (send(id, content, size*sizeof(char),0) == -1) { perror("[-]Error send"); shutdown(id, 2); shutdown(datas.dS,2); exit(0); }
    } 

    free(content);
}

/**
 * @brief Reads the list of available commands in a file
 * 
 * @param content The place to put the content
 * @return The list of file
 */
char* helpMessage(char* content) {
    FILE *f;
    char c;
    f = fopen("listCommand.txt", "rt");
    int i = 0;
    while((c=fgetc(f))!=EOF){ content[i] = c; i++; } // There are some lines
    fclose(f);
    return content;
}

/**
 * @brief Read a file an update actual channels
 * 
 * @param data Information used for server management
 */
void channelList(data* data){
    char* content = malloc(1000*sizeof(char));

    FILE *f;
    char c;
    f = fopen("listChannel.txt", "rt");
    int i = 0;
    while((c=fgetc(f))!=EOF){ content[i] = c; i++; } // There are some lines
    fclose(f);

    char d[] = "\n";
    char *p = strtok(content, d);
    int j = 0;
    while((p != NULL) && (j<20)) { // Update channel array
        strcpy(data->arrayChannelName[j],p);
        p = strtok(NULL, d);
        j++;
    }

}

/**
 * @brief Save actual channels in a file
 * 
 * @param data Information used for server management
 */
void saveChannels(data* data) {
    FILE *f;
    char c;
    f = fopen("listChannel.txt", "w");
    int i = 0;
    while(i<20){ // Array reading not finish
        if (strcmp(data->arrayChannelName[i],"empty")!=0) {
            fwrite(data->arrayChannelName[i], 1, strlen(data->arrayChannelName[i]), f);   
            fwrite("\n", 1, 1, f);   
        }
        i++; 
    }
    fclose(f);
}

/**
 * @brief Returns the list of online clients
 * 
 * @param content The place to put the content
 * @param data Information used for server management
 * @return The list
 */
char * listClient(char* content, data* data) { 
    int iterator1 = 0;
    for (int i=0;i<20;i++) { // Each cell
        if (data->arrayId[i] != (int*)-1) {
            int iterator2 = 0;
            while((data->arrayName[i])[iterator2] != '\0') { // Reading of client name
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

/**
 * @brief // Returns the list of actual channels
 * 
 * @param content The place to put the content
 * @param data Information used for server management
 * @param id The if of the client
 * @return The list
 */
char * listClientChannel(char* content, data* data, int id) {
    int iterator1 = 0;
    for (int i=0;i<20;i++) { // Each cell
        if ((data->arrayIdChannel[i] == data->arrayIdChannel[id]) && (strcmp(data->arrayName[i],"empty") != 0)) {
            int iterator2 = 0;
            while((data->arrayName[i])[iterator2] != '\0') { // Reading of channel name
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

/**
 * @brief See if a username is already in use
 * 
 * @param data Information used for server management
 * @param pseudo The nickname to check
 * @return 1 if the nickname is invalid, 0 otherwise
 */
int checkPseudo(data* data, char* pseudo) { 
    int res = testRegex("^[a-zA-Z0-9]{5}",pseudo,0);
    if (res != 1) { res = testRegex("(admin)",pseudo,1); }
    if (res != 1) { // Not already in use
        for (int i=0;i<20;i++) {
            if (strcmp(data->arrayName[i],pseudo) == 0) { res=1; }
        }
    }
    return res;
}

/**
 * @brief See if a content contains a regex
 * 
 * @param regex The regex to check
 * @param content The content to check
 * @param param To change the result
 * @return 0 if ok, 1 otherwise
 */
int testRegex(char* regex, char* content, int param) {
    regex_t reg;
    int reti = regcomp(&reg, regex, REG_EXTENDED);
    reti = regexec(&reg, content, 0, NULL, 0);
    if (!reti && param == 1) { // No match
        regfree(&reg);
        return 1;
    }
    if ((reti == REG_NOMATCH) && param == 0) { // Match
        regfree(&reg);
        return 1;
    }
    regfree(&reg);
    return 0;
}

/**
 * @brief See if a channel already exists
 * 
 * @param data Information used for server management
 * @param channel The channel name to check
 * @return 1 if the channel exists, 0 otherwise
 */
int checkChannel(data* data, char* channel) { // 
    for (int i=0;i<20;i++) {
        if (strcmp(data->arrayChannelName[i],channel) == 0) { return 1; }
    }
    return 0;
}

/**
 * @brief Create a new file
 * 
 * @param sockfd Transmission socket number
 * @param filename Name of the file
 */
void write_file(int sockfd, char* filename) { 
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

/**
 * @brief Send a file
 * 
 * @param fp The file to send
 * @param sockfd Transmission socket number
 */
void send_file(FILE *fp, int sockfd) { 
    int n;
    char data[SIZE] = {0};
    while(fread(data, sizeof(char), SIZE, fp)!= (unsigned long)NULL) { // There are some data
        if (send(sockfd, data, sizeof(data), 0) == -1) { perror("[-]Error in sending file."); exit(1);}
        bzero(data, SIZE);
    }
}

/**
 * @brief Create a channel
 * 
 * @param channel The name of the channel
 * @param data Information used for server management
 */
void createChannel(char* channel, data* data, int index){
    int id = nextEmptyChannel(data);
    if (id != -1) { 
        if ((checkChannel(data, channel) == 0) && (strcmp(channel,"empty") != 0) && (testRegex("^[a-zA-Z0-9]{4}",channel,0) != 1)) { // Channel does not exist
            strcpy(datas.arrayChannelName[id],channel);
            personalMessage("## Channel created ##", data->arrayName[index], data,index);
            return;
        }
        personalMessage("## Channel name invalid ##", data->arrayName[index], data,index);
        return;
    }   
    personalMessage("## Too many channels ##", data->arrayName[index], data,index);
}

/**
 * @brief Delete a channel
 * 
 * @param channel The name of the channel
 * @param data Information used for server management
 */
void deleteChannel(char* channel, data* data, int index){
    for (int i=0; i<20; i++) {
        if ((strcmp(data->arrayChannelName[i],channel) == 0) && (strcmp(channel,"public") != 0)) { 
            data->arrayIdChannel[i] = 0;
            strcpy(data->arrayChannelName[i],"empty");
            personalMessage("## Channel deleted ##", data->arrayName[index], data,index);
            return;
        }
    }
    personalMessage("## Channel name invalid ##", data->arrayName[index], data,index);
}

/**
 * @brief Retrieves the username corresponding to an id
 * 
 * @param channel The name of the channel
 * @param data Information used for server management
 * @return The id of the channel, 0 if not found
 */
int nameToIdChannel(char* channel, data* data) {
    for (int i=0;i<20;i++) {
        if (strcmp(data->arrayChannelName[i],channel) == 0) { return i; }
    }
    return -1;
}

/**
 * @brief Connect to a channel
 * 
 * @param channel The name of the channel
 * @param index The index of the client
 * @param data Information used for server management
 */
void connectChannel(char* channel, int index, data* data){
    if (checkChannel(data, channel) == 1) {
        strtok(channel,"\n");
        data->arrayIdChannel[index] = (int*)(size_t)nameToIdChannel(channel,data);
        personalMessage("## Connected to the new channel ##", data->arrayName[index], data,index);
        return;
    }
    personalMessage("## Channel invalid ##", data->arrayName[index], data,index);
}

/**
 * @brief Returns the list of channels
 * 
 * @param content The place to put the content
 * @param data Information used for server management
 * @return The list
 */
char* listChannels(char* content, data* data){
    int iterator1 = 0;
    for (int i=0;i<20;i++) { // Each cell
        if (strcmp(data->arrayChannelName[i],"empty")!=0) {
            int iterator2 = 0;
            while((data->arrayChannelName[i])[iterator2] != '\0') { // Reading of channel name
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
