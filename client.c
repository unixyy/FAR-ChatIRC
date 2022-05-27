
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include "funcCli.h"

#define haveToStop "@quit"

pthread_t my_thread;

void  INThandler(int); // Takes care of signal management

/**
 * @brief Manages the connection to the server and the sending of messages
 * 
 * @param argc The number of arguments
 * @param argv The ip address and port number of the server
 * @return A default 0
 */
int main(int argc, char *argv[]) {

  signal(SIGINT, INThandler); // Manages the Ctrl-C

  datas.stop=0;
  datas.dS = socket(PF_INET, SOCK_STREAM, 0); // Create a socket
  if (datas.dS == -1) { perror("[-]Error socket"); exit(0); }

  struct sockaddr_in aS;
  aS.sin_family = AF_INET;
  inet_pton(AF_INET,argv[1],&(aS.sin_addr)); // Server ip adress
  aS.sin_port = htons(atoi(argv[2])); // Server port
  socklen_t lgA = sizeof(struct sockaddr_in);

  if (connect(datas.dS, (struct sockaddr *) &aS, lgA) == -1) { perror("[-]Error connect"); shutdown(datas.dS,2); exit(0); } // Connection to the server

  pthread_create(&my_thread, NULL, (void*)Receive, &datas); // Creates a thread that manages the reciving of messages

  int stop = 0;
  printf("\033[31;1;%dmConnection...\n\n\033[0m",1);

  while (!stop && !datas.stop) {
    char *m = (char *)malloc(sizeof(char)*30);
    fgets(m, sizeof(char)*30, stdin); // Message to send
    printf("\n");
    strtok(m,"\n");

    if (isCommand(m)) { executeCommand(m,&sfiles,argv[1]); } 
    else {
      int taille = strlen(m);
      if (send(datas.dS, &taille, sizeof(int), 0) == -1) { perror("[-]Error send"); shutdown(datas.dS,2); exit(0); } // Sends the message size to the server
      if (send(datas.dS, m, strlen(m) , 0) == -1) { perror("[-]Error send"); shutdown(datas.dS,2); exit(0); } // Sends the message to the server

      strtok(haveToStop,"\0");
      if (strcmp(m, haveToStop) == 0) { stop=1; } 
      free(m);
    }
  }
  printf("\033[31;1;1mDisconnection...\n\033[0m");
  pthread_kill(my_thread, SIGTERM); // Kill the receive thread
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
  if (c == 'y' || c == 'Y') { printf("\033[31;1;1mDisconnection...\n\033[0m"); exit(0); }    
  else { signal(SIGINT, INThandler); }
  getchar(); // Get new line character
}
