
struct data {
    int dS;
    int stop;
}datas;
typedef struct data data;

struct sfile {
    char* ip;
    char* filename;
}sfiles;
typedef struct sfile sfile;

void * Receive(data* datas); // Thread for receiving a message

int listFile(); // Print files list of current directory

void file(sfile* sfiles); // Sending a file to the server

void send_file(FILE *fp, int sockfd); // Send the content of a file

int isCommand(char* msg); // Determines if the message contains a command

char** getCommand(char* msg); // Separates the information in a message

void executeCommand(char* content, sfile* sfiles, char* ip); // Executes a particular command

void downloadFile(sfile* sfiles); // Receiving a file from the server

void write_file(int sockfd, char* filename); // Create the new file
