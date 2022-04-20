
struct sockaddr_in aC;

struct data {
    int dS;
    char* arrayCli[20][2];
    int indexCli;
    int id;
} datas;
typedef struct data data;

int actualIndex(data* data);

void * receiveSend(data* datas);

int nextEmpty(data* data);

void deleteUser(data* data, char* id);