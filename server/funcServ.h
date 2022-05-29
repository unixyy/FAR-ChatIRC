
#ifdef __APPLE__
    #include <dispatch/dispatch.h>
#else
    #include <semaphore.h>
#endif

struct sockaddr_in aC;

struct rk_sema { // Create a sempahore
    #ifdef __APPLE__
        dispatch_semaphore_t    sem;
    #else
        sem_t                   sem;
    #endif
};

static inline void rk_sema_init(struct rk_sema *s, uint32_t value) { // Initialize a semaphore
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
        
        sem_wait(&s->sem); 
        
    #endif
}

static inline void rk_sema_post(struct rk_sema *s) { // Unlock a semaphore
    #ifdef __APPLE__
        dispatch_semaphore_signal(s->sem);
    #else
        sem_post(&s->sem);
    #endif
}

// Information used for server management
struct data {
    int dS;
    int* arrayId[20];
    char* arrayName[20];
    int* arrayIdChannel[20];
    char* arrayChannelName[20];
    int actualId;
    struct rk_sema* s;
    pthread_t* threadToClose[24];
    int* isClose[24];
    pthread_mutex_t mutex;
    int close;
} datas;
typedef struct data data;

void personalMessage(char* msg, char* username, data* data, int index); // Sends a message to the user who executed the command

int actualIndex(data* data); // Returns the index of a cell corresponding to a socket id

int isCommand(char* msg); // Determines if the received message contains a command

char** getCommand(char* msg); // Separates the information in a message

void executeCommand(char* content, data* data, int id); // Executes a particular command

void privateMessage(char* msg, char* username, data* data, int id); // Sends a private message to another user

char * listClient(char* content, data* data); // Returns the list of online clients

char* helpMessage(char* content); // Reads the list of available commands in a file

void broadcast(data *datas, char *text1, char *text2, int index); // Send a message to all the clients

int nameToId(char* username, data* data); // Retrieves the username corresponding to an id

char* idToName(int id, data* data); // Retrieves the correct id for a username

int nextEmpty(data* data); // Calculate the index of the first empty cell of the name array 

int nextEmptyChannel(data* data); // Calculate the index of the first empty cell of the channel array

void deleteUser(data* data, int id); // Removes a user from the user list

int checkPseudo(data* data, char* pseudo); // See if a username is already in use

int checkChannel(data* data, char* channel); // See if a channel name is already in use

char* listFile(char* content); // Print files list of the server

void write_file(int sockfd, char* filename); // Create a new file

void send_file(FILE *fp, int sockfd); // Send the file

void createChannel(char* channel, data* data, int index); // Create a channel

void deleteChannel(char* channel, data* data, int index); // Delete a channel

void connectChannel(char* channel,int index, data* data); // Connect to a channel

char* listChannels(char* content, data* data); // Returns the list of channels

int nameToIdChannel(char* channel, data* data); // Retrieves the username corresponding to an id

void messageChannel(data* datas, char* msg, int index); // Send a message in the current channel

char * listClientChannel(char* content, data* data, int id); // Returns the list of actual channels

void channelList(data* data); // Read a file an update actual channels

void saveChannels(data* data); // Save actual channels in a file

void report(char* id, char* text1, char* text2); // Report a situation to an admin

void adminCommand(char* content, data* data); // Executes a particular admin command 

void kick(data* data, char* name); // Kick a client from the server

void adminPrivateMessage(char* msg, char* username, data* data); // Sends a private message to another user by an admin

void adminBroadcast(data* datas, char* text1,char* text2); // Sending a message to all clients by an admin

int testRegex(char* regex, char* content, int param); // See if a content contains a regex

int indexClient(data* data, int id); // Calculate the index corresponding to a client id
