
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
    pthread_t* threadToClose[20];
    int* isClose[20];
}datas;
typedef struct data data;

int actualIndex(data* data); // Returns the index of a cell corresponding to a socket id

int isCommand(char* msg); // Determines if the received message contains a command

char** getCommand(char* msg); // Separates the information in a message

void executeCommand(char* content, data* data, int id); // Executes a certain command

void privateMessage(char* msg, char* username, data* data, int id); // Sends a private message to another user

void personalMessage(char* msg, char* username, data* data, int index); // Sends a message to the user who executed the command

char * listClient(char* content, data* data); // Returns the list of online clients

char* helpMessage(char* content); // Reads the list of available commands in a file

int nameToId(char* username, data* data); // Retrieves the username corresponding to an id

char* idToName(int id, data* data); // Retrieves the correct id for a username

void * receiveSend(data* datas, pthread_mutex_t* mutex); // Thread for receiving and sending a message

int nextEmpty(data* data); // Returns the index of the first empty cell of an array

void deleteUser(data* data, int id); // Removes a user from the user list

int checkPseudo(data* data, char* pseudo); // See if a username is already in use

void * closeThread(data* datas); // Thread for closing a receiveSend thread

char* listFile(char* content); // Print files list of the server

void write_file(int sockfd, char* filename);

void file(char* filename);

void downloadFile(char* filename);

void send_file(FILE *fp, int sockfd);

static inline void rk_sema_init(struct rk_sema *s, uint32_t value) { // initialize a semaphore
    #ifdef __APPLE__
        dispatch_semaphore_t *sem = &s->sem;
        *sem = dispatch_semaphore_create(value);
    #else
        sem_init(&s->sem, 0, value);
    #endif
}

static inline void rk_sema_wait(struct rk_sema *s) { // Lock a semaphore
    #ifdef __APPLE__
        dispatch_semaphore_wait(s->sem, DISPATCH_TIME_FOREVER);
    #else
        int r;
        do { r = sem_wait(&s->sem);} 
        while (r == -1 && errno == EINTR);
    #endif
}

static inline void rk_sema_post(struct rk_sema *s) { // Unlock a semaphore
    #ifdef __APPLE__
        dispatch_semaphore_signal(s->sem);
    #else
        sem_post(&s->sem);
    #endif
}
