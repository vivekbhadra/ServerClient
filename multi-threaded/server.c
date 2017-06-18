#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include "common.h"

#define PORT 4444
#define BUF_SIZE 2000
#define CLADDR_LEN 100


struct threadData {
	struct sockaddr_in *cl_addr;
	int sock;
	int len;
	char clientAddr[CLADDR_LEN];
};
//
// This will handle connection for each client
//
void *connection_handler(void *arg)
{
	char buffer[BUF_SIZE];
	int ret;
	message_t *message;
	struct threadData *tData = (struct threadData *)arg;

	for (;;) {
	    memset(buffer, 0, BUF_SIZE);
	    fprintf(stdout, "Hello World I am new connection\n");
	    ret = recvfrom(tData->sock, buffer, BUF_SIZE, 0, (struct sockaddr *) tData->cl_addr, &tData->len);
	    if(ret < 0) {
	    	fprintf(stderr, "Error receiving data: %s\n", gai_strerror(ret));
	    	exit(EXIT_FAILURE);
	    }
	    message = (message_t *)buffer;
	    fprintf(stdout, "Received %s request from %s\n", message->req ? "WRITE" : "READ", tData->clientAddr);

	    switch(message->req) {
	    case READ_REQ:
	    	strcpy(message->message, "Hi");
	    	ret = sendto(tData->sock, message->message, BUF_SIZE, 0, (struct sockaddr *) tData->cl_addr, tData->len);
	        if (ret < 0) {
	    	    fprintf(stderr, "Error sending data!\n");
	    	    exit(EXIT_FAILURE);
	    	}
	        fprintf(stdout, "Sent data to %s: %s\n", tData->clientAddr, message->message);
	    	break;
	    case WRITE_REQ:
	    	break;
	    }
	}
	return 0;
}

int main() {
 struct sockaddr_in addr, cl_addr;
 int sockfd, len, ret, newsockfd;
 char buffer[BUF_SIZE];
 pid_t childpid;
 char clientAddr[CLADDR_LEN];
 pthread_t thread_id;
 int client_sock, c;
 struct threadData *threadParams;

 sockfd = socket(AF_INET, SOCK_STREAM, 0);
 if (sockfd < 0) {
  fprintf(stderr, "Error creating socket: %s\n", gai_strerror(sockfd));
  exit(EXIT_FAILURE);
 }

 memset(&addr, 0, sizeof(addr));
 addr.sin_family = AF_INET;
 addr.sin_addr.s_addr = INADDR_ANY;
 addr.sin_port = PORT;

 ret = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));
 if (ret < 0) {
	 fprintf(stderr, "Error binding: %s\n", gai_strerror(ret));
	 exit(EXIT_FAILURE);
 }

 fprintf(stdout, "Waiting for a connection...\n");
 listen(sockfd, 5);

 for (;;) {
  len = sizeof(cl_addr);
  newsockfd = accept(sockfd, (struct sockaddr *) &cl_addr, &len);
  if (newsockfd < 0) {
	  fprintf(stderr, "Error accepting connection: %s\n", gai_strerror(ret));
	  exit(EXIT_FAILURE);
  }

  fprintf(stdout, "Connection accepted...\n");

  inet_ntop(AF_INET, &(cl_addr.sin_addr), clientAddr, CLADDR_LEN);
  /*
   * Create a new child process to handle the new client.
   * */

  threadParams = malloc(sizeof(struct threadData));
  if (!threadParams) {
  	  fprintf(stderr, "Error accepting connection: %s\n", gai_strerror(ret));
  	  exit(EXIT_FAILURE);
  } else {
	  threadParams->cl_addr = (struct sockaddr_in *) &cl_addr;
	  threadParams->sock = newsockfd;
	  threadParams->len = len;
	  strncpy(threadParams->clientAddr, clientAddr, CLADDR_LEN);
  }

  if(pthread_create( &thread_id, NULL,  connection_handler, (void*) threadParams) < 0) {
	  fprintf(stderr, "Error Client handler function couldn't be crated.\n");
	  exit(EXIT_FAILURE);
  }
 }
 exit(EXIT_SUCCESS);
}
