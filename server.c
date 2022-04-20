
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

  data datas;


  listen(dS, 7); // Setup the socket in listening mode
  printf("Listening mode\n");
  printf("c'est chelou\n");
  datas.dS = dS;
  printf("test2\n");
  datas.indexCli = 0;
  printf("test1\n");
  datas.id = 0;
  printf("test\n");
  for (int l = 0; l < 20; l++) {
    for (int c = 0;c<2;c++) {
      datas.arrayCli[l][c] = calloc(40,sizeof(char));
      strcpy(datas.arrayCli[l][c],"nonEmpty");
      printf("%s\n",datas.arrayCli[l][c]);
    }
  }
  printf("passed remplissage\n");

  socklen_t lg = sizeof(struct sockaddr_in);

  while (1) {

    if (datas.indexCli < 19) { // If there is too many clients

      printf("ok\n");
      int next = nextEmpty(&datas);
      printf("%d\n",next);

      datas.arrayCli[next][0] = (char *) accept(dS, (struct sockaddr*) &aC,&lg) ; // Accept a client
      if (datas.arrayCli[datas.indexCli] == -1) { perror("Error accept"); shutdown(dS, 2); exit(0);
      }
      datas.id = datas.arrayCli[next][0];
      datas.indexCli++;
      printf("Connected client\n");

      pthread_t thread[NB_THREADS];
      pthread_create(&thread[datas.indexCli], NULL, receiveSend, &datas); // Creates a thread that manages the relaying of messages

     }
      /*char *buf;
      fgets(buf, sizeof(buf), stdin); // Message to send
      */
  }

  shutdown(datas.dS, 2); // Close the socket
  printf("End of program");

}
