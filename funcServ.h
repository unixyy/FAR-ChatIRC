
struct sockaddr_in aC;

struct data {
    int dS;
    int arrayCli[20];
    int indexCli;
} datas;
typedef struct data data;

void * receiveSend(data* datas);
