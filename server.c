
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <pthread.h>
#include <signal.h>
#include "funcServ.h"

#define NB_THREADS 21


  pthread_t thread[NB_THREADS];

void  INThandler(int sig)
{
     char  c;

     signal(sig, SIG_IGN);
     printf("\nOUCH, did you hit Ctrl-C?\n"
            "Do you really want to quit? [y/n] ");
     c = getchar();
     if (c == 'y' || c == 'Y') {
            shutdown(datas.dS, 2); // Close the socket
  printf("End of program\n");
    for (int i=0;i<21;i++) {
    pthread_kill(thread[i], SIGTERM);
  }
  exit(0);
     }
     else
          signal(SIGINT, INThandler);
     getchar(); // Get new line character
}

static inline void
rk_sema_init(struct rk_sema *s, uint32_t value)
{
#ifdef __APPLE__
    dispatch_semaphore_t *sem = &s->sem;

    *sem = dispatch_semaphore_create(value);
#else
    sem_init(&s->sem, 0, value);
#endif
}

static inline void
rk_sema_wait(struct rk_sema *s)
{

#ifdef __APPLE__
    dispatch_semaphore_wait(s->sem, DISPATCH_TIME_FOREVER);
#else
    int r;

    do {
            r = sem_wait(&s->sem);
    } while (r == -1 && errno == EINTR);
#endif
}

int main(int argc, char *argv[]) {

  signal(SIGINT, INThandler);

  printf("Start of the server\n");
  
  int dS = socket(PF_INET, SOCK_STREAM, 0); // Create a socket
  if (dS == -1) { perror("Socket error"); exit(0); }
  printf("Socket created\n");

  struct sockaddr_in ad;
  ad.sin_family = AF_INET;
  ad.sin_addr.s_addr = INADDR_ANY;
  ad.sin_port = htons(atoi(argv[1])); // Setup the server port with the number passed in parameter

  if (bind(dS, (struct sockaddr*)&ad, sizeof(ad)) == -1){ perror("Error bind"); exit(0); } // Name the socket
  printf("Socket named\n");

  listen(dS, 7); // Setup the socket in listening mode
  printf("Listening mode\n");

  struct rk_sema s;

  datas.dS = dS;
  datas.actualId = 0;
  datas.s = &s;
  for (int i = 0; i < 20; i++) {
      datas.arrayName[i] = calloc(40,sizeof(char));
      datas.arrayId[i] = -1;
      datas.isClose[i] = 0;
      strcpy(datas.arrayName[i],"empty");
  }

  socklen_t lg = sizeof(struct sockaddr_in);

  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

  rk_sema_init(datas.s, 20);

  while (1) {


    rk_sema_wait(datas.s);

      int next = nextEmpty(&datas);
      
      int test = accept(dS, (struct sockaddr*) &aC,&lg) ; // Accept a client
      if (test== -1) { perror("Error accept"); shutdown(dS, 2); exit(0);
      }
      pthread_mutex_lock(&mutex);
      datas.arrayId[next] = test;
      datas.actualId = test;
      printf("Connected client\n");
      
      pthread_create(&thread[next], NULL, receiveSend, &datas); // Creates a thread that manages the relaying of messages
      pthread_create(&thread[20], NULL, closeThread, &datas); // Creates a thread that manages the relaying of messages

      pthread_mutex_unlock(&mutex);

  }

  return 0;

}

