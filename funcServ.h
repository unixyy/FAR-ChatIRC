
struct sockaddr_in aC;

struct data {
    int dS;
    int* arrayId[20];
    char* arrayName[20];
    int actualId;
    int numberCli;
}datas;
typedef struct data data;

int actualIndex(data* data);

int isCommand(char* msg);

char** getCommand(char* msg);

void executeCommand(char* content, data* data, int id);

void privateMessage(char* msg, char* username, data* data, int id);

void privateMessage2(char* msg, char* username, data* data, int index);

char * listClient(char* content, data* data);

char* helpMessage(char* content);

int nameToId(char* username, data* data);

char* idToName(int id, data* data);

void * receiveSend(data* datas);

int nextEmpty(data* data);

void deleteUser(data* data, int id);