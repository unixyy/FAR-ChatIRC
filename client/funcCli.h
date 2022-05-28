
// Information to communicate with the server
struct data { int dS; int stop; } datas;
typedef struct data data;

// File management structure
struct sfile { char* ip; char* filename; } sfiles; 
typedef struct sfile sfile;

int listFile(); // Print files list of current directory

void send_file(FILE *fp, int sockfd); // Send the content of a file

int isCommand(char* msg); // Determines if the message contains a command

char** getCommand(char* msg); // Separates the information in a message

void executeCommand(char* content, sfile* sfiles, char* ip); // Executes a particular command

void write_file(int sockfd, char* filename); // Create a new file with a content
