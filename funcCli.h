
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

void file(sfile* sfiles);

void send_file(FILE *fp, int sockfd);

int isCommand(char* msg);

char** getCommand(char* msg);

void executeCommand(char* content, sfile* sfiles, char* ip);
