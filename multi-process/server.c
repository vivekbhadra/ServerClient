#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#define PORT 4444
#define BUF_SIZE 2000
#define CLADDR_LEN 100

int main() {
 struct sockaddr_in addr, cl_addr;
 int sockfd, len, ret, newsockfd;
 char buffer[BUF_SIZE];
 pid_t childpid;
 char clientAddr[CLADDR_LEN];

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
  if ((childpid = fork()) == 0) {

  close(sockfd);
    //stop listening for new connections by the main process.
    //the child will continue to listen.
    //the main process now handles the connected client.

   for (;;) {
    memset(buffer, 0, BUF_SIZE);
    ret = recvfrom(newsockfd, buffer, BUF_SIZE, 0, (struct sockaddr *) &cl_addr, &len);
    if(ret < 0) {
    	fprintf(stderr, "Error receiving data: %s\n", gai_strerror(ret));
    	exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Received data from %s: %s\n", clientAddr, buffer);

    ret = sendto(newsockfd, buffer, BUF_SIZE, 0, (struct sockaddr *) &cl_addr, len);
    if (ret < 0) {
    	fprintf(stderr, "Error sending data!\n");
    	exit(EXIT_FAILURE);
    }
    printf("Sent data to %s: %s\n", clientAddr, buffer);
   }
  }
  close(newsockfd);
 }
 exit(EXIT_SUCCESS);
}
