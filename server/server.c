
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <pthread.h>
#include <signal.h>
#include "funcServ.h"
#include "threadServ.h"

#define NB_THREADS 24

pthread_t thread[NB_THREADS];

void  INThandler(int sig); // Takes care of signal management

/**
 * @brief Manages socket creation and client connection
 * 
 * @param argc The number of arguments
 * @param argv An available port number
 * @return A default 0
 */
int main(int argc, char *argv[]) {

  signal(SIGINT, INThandler);
  
  int dS = socket(PF_INET, SOCK_STREAM, 0); // Create a socket
  if (dS == -1) { perror("[-]Socket error"); exit(0); }

  struct sockaddr_in ad;
  ad.sin_family = AF_INET;
  ad.sin_addr.s_addr = INADDR_ANY;
  ad.sin_port = htons(atoi(argv[1])); // Setup the server port with the number passed in parameter

  if (bind(dS, (struct sockaddr*)&ad, sizeof(ad)) == -1){ perror("[-]Error bind"); exit(0); } // Name the socket

  listen(dS, 7); // Setup the socket in listening mode

  struct rk_sema s;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

  // Setting up the basic elements for customer management
  datas.dS = dS;
  datas.actualId = 0;
  datas.s = &s;
  datas.mutex = mutex;
  datas.close = 0;
  for (int i = 0; i < 20; i++) {
      datas.arrayName[i] = calloc(40,sizeof(char));
      datas.arrayChannelName[i] = calloc(40,sizeof(char));
      datas.arrayIdChannel[i] = (int*)0;
      datas.arrayId[i] = (int*)-1;
      datas.isClose[i] = 0;
      strcpy(datas.arrayName[i],"empty");
      strcpy(datas.arrayChannelName[i],"empty");
  }
  channelList(&datas); // Recreate the channels stored in a file

  socklen_t lg = sizeof(struct sockaddr_in);

  rk_sema_init(datas.s, 20);

  printf("\033[34;1;1mSTART OF THE SERVER\n\033[0m");

  pthread_create(&thread[23], NULL, (void*)file, &datas); // Creates a thread that manages the reciving of files
  pthread_create(&thread[22], NULL, (void*)downloadFile, &datas); // Creates a thread that manages the sending of files
  pthread_create(&thread[21], NULL, (void*)admin, &datas); // Creates a thread that manages the administrator session
  pthread_create(&thread[20], NULL, (void*)closeThread, &datas); // Creates a thread that manages the closing of receiveSend threads

  while (1) {
    rk_sema_wait(datas.s);
    int next = nextEmpty(&datas);
      
    int idClient = accept(dS, (struct sockaddr*) &aC,&lg) ; // Accept a client
    if (idClient== -1) { perror("Error accept"); shutdown(dS, 2); exit(0);}
      
    pthread_mutex_lock(&datas.mutex);
    datas.arrayId[next] = (int*)(size_t)idClient;
    datas.actualId = idClient;
    pthread_create(&thread[next], NULL, (void*)receiveSend, &datas); // Creates a thread that manages the relaying of messages
    pthread_mutex_unlock(&datas.mutex);
  }
  return 0;
}

/**
 * @brief Takes care of signal management
 * 
 * @param sig Signal to be processed
 */
void  INThandler(int sig) {
  char  c;
  signal(sig, SIG_IGN);
  printf("\033[34;1;1m\nOUCH, did you hit Ctrl-C ?\n""Do you really want to quit ? [y/n] \033[0m");
  c = getchar();
  if (c == 'y' || c == 'Y') { datas.close = 1; pthread_join(thread[20],NULL); saveChannels(&datas); shutdown(datas.dS, 2); printf("\033[34;1;1m\nEND OF PROGRAM\n\033[0m"); exit(0);} 
  else { signal(SIGINT, INThandler); }
  getchar(); // Get new line character
}
