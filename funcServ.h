
struct sockaddr_in aC;

struct data {
    int dS;
    int* arrayId[20]; // Problem here
    char* arrayName[20];
    int actualId;
    int numberCli;
}datas;
typedef struct data data;

int actualIndex(data* data);

void * receiveSend(data* datas);

int nextEmpty(data* data);

void deleteUser(data* data, int id);