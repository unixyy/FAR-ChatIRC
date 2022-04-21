
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <pthread.h>
#include "funcServ.h"

#define NB_THREADS 20

int main(int argc, char *argv[]) {

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

  datas.dS = dS;
  datas.numberCli = 0;
  datas.actualId = 0;
  for (int i = 0; i < 20; i++) {
      datas.arrayName[i] = calloc(40,sizeof(char));
      datas.arrayId[i] = -1;
      strcpy(datas.arrayName[i],"empty");
  }

  socklen_t lg = sizeof(struct sockaddr_in);

  while (1) {

    if (datas.numberCli < 19) {

      int next = nextEmpty(&datas);
      
      int test = accept(dS, (struct sockaddr*) &aC,&lg) ; // Accept a client
      if (test== -1) { perror("Error accept"); shutdown(dS, 2); exit(0);
      }
      datas.arrayId[next] = test;
      datas.actualId = test;
      datas.numberCli++;
      printf("Connected client\n");
      
      pthread_t thread[NB_THREADS];
      pthread_create(&thread[next], NULL, receiveSend, &datas); // Creates a thread that manages the relaying of messages

     }

  }

  shutdown(datas.dS, 2); // Close the socket
  printf("End of program");

}
