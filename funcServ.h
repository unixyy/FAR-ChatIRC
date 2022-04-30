#ifdef __APPLE__
#include <dispatch/dispatch.h>
#else
#include <semaphore.h>
#endif

struct sockaddr_in aC;

struct rk_sema {
#ifdef __APPLE__
    dispatch_semaphore_t    sem;
#else
    sem_t                   sem;
#endif
};

struct data {
    int dS;
    int* arrayId[20];
    char* arrayName[20];
    int actualId;
    struct rk_sema* s;
    struct rk_sema* s1;
    pthread_t* threadToClose[20];
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

void * receiveSend(data* datas, pthread_mutex_t* mutex);

int nextEmpty(data* data);

void deleteUser(data* data, int id);

int checkPseudo(data* data, char* pseudo);

static inline void rk_sema_init(struct rk_sema *s, uint32_t value);

static inline void rk_sema_wait(struct rk_sema *s);

static inline void rk_sema_post(struct rk_sema *s);

void * closeThread(data* datas);
