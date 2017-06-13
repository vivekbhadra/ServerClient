#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>


#define MAX_CON 5
void * thread_func (void *thread_data);
struct thread_data {    /* Used as argument to thread_start() */
    int    clisock;
};


/* Define functions here */
void printAddressDetails(const struct sockaddr *addr)
{
	void *numAddr;
	char buff[INET6_ADDRSTRLEN];
	if (NULL == addr) {
		fprintf(stdout, "Null address pointer\n");
		return;
	}

	in_port_t port;
	switch(addr->sa_family) {
	case AF_INET:
		numAddr = &((struct sockaddr_in *)addr)->sin_addr;
		port = ntohs(((struct sockaddr_in *)addr)->sin_port);
		break;

	case AF_INET6:
	    numAddr = &((struct sockaddr_in6 *)addr)->sin6_addr;
	    port = ntohs(((struct sockaddr_in6 *)addr)->sin6_port);
	    break;
	default:
		fprintf(stdout, "Address type not recognised\n");
		return;
	}

	if (inet_ntop(addr->sa_family, numAddr, buff, sizeof(buff)) == NULL) {
		fprintf(stdout, "Failed to get address details\n");
		return;
	}

	fprintf(stdout, "%s:%d\n", buff, port);
}
//int createServerSocket(const char *);

int main( int argc, char *argv[] ) {
   int sockfd, portno, clilen;
   char buffer[256];
   struct sockaddr_in serv_addr, cli_addr;
   int n, pid;
   struct addrinfo hints;
   int sfd, s, newsockfd;
   struct addrinfo *result, *rp;
   struct sockaddr_storage cliaddr;
   socklen_t cliaddrlen;
   struct thread_data *tdata;
   pthread_t threadid;

   memset(&hints, 0, sizeof(struct addrinfo));

   hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
   hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
   hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
   hints.ai_protocol = IPPROTO_TCP;          /* Any protocol */
   hints.ai_canonname = NULL;
   hints.ai_addr = NULL;
   hints.ai_next = NULL;

   if (argc != 2) {
           fprintf(stderr, "Usage: %s port\n", argv[0]);
           exit(EXIT_FAILURE);
   }

   s = getaddrinfo(NULL, argv[1], &hints, &result);
   if (s != 0) {
       fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
       exit(EXIT_FAILURE);
   }

   /* Try each address until we successfully bind().
    * If socket() (or bind()) fails, we (close the socket
    * and) try the next address.
    * */
   for (rp = result; rp != NULL; rp = rp->ai_next) {
	   fprintf(stdout, "Loop\n");
	   sfd = socket(rp->ai_family, rp->ai_socktype,
                   rp->ai_protocol);
       if (sfd == -1)
           continue;

       if ((bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) &&
    		   listen(sfd, MAX_CON) == 0) {
    	   struct sockaddr_storage localAddress;
    	   socklen_t addrSize = sizeof(localAddress);
    	   if (getsockname(sfd, (struct sockaddr *)&localAddress, &addrSize) < 0) {
    		   fprintf(stderr, "Could not get socket name\n");
    		   exit(EXIT_FAILURE);
    	   }
    	   printAddressDetails((struct sockaddr *)&localAddress);
           break;                  /* Success */
       }
       close(sfd);
   }

   if (rp == NULL) {               /* No address succeeded */
       fprintf(stderr, "Could not bind\n");
       exit(EXIT_FAILURE);
   }

   freeaddrinfo(result);           /* No longer needed */
   
   clilen = sizeof(cli_addr);
   
   while (1) {
	   newsockfd = accept(sfd, (struct sockaddr *) &cli_addr, &clilen);
       if (newsockfd < 0) {
    	   fprintf(stderr, "Failed to accept client\n");
    	   exit(EXIT_FAILURE);
       }

       printAddressDetails((struct sockaddr *) &cli_addr);

        tdata = (struct thread_data *) malloc(sizeof(struct thread_data));
        if (!tdata) {
        	fprintf(stderr, "Failed to create client data\n");
            exit(EXIT_FAILURE);
        }

        tdata->clisock = newsockfd;
        threadid = pthread_create(&threadid, NULL, thread_func, tdata);


#if 0
      newsockfd = accept(sfd, (struct sockaddr *) &cli_addr, &clilen);
		
      if (newsockfd < 0) {
         perror("ERROR on accept");
         exit(1);
      }
      
      /* Create child process */
      pid = fork();
		
      if (pid < 0) {
         perror("ERROR on fork");
         exit(1);
      }
      
      if (pid == 0) {
         /* This is the client process */
         close(sockfd);
         doprocessing(newsockfd);
         exit(0);
      }
      else {
         close(newsockfd);
      }
#endif
   } /* end of while */
} 

void * thread_func (void *tdata) {
   int n;
   char buffer[256];
   bzero(buffer,256);
   int clisock;

   clisock = ((struct thread_data *)tdata)->clisock;
   n = read(clisock,buffer,255);
   
   if (n < 0) {
      perror("ERROR reading from socket");
      exit(1);
   }
   
   printf("Here is the message: %s\n",buffer);
   n = write(clisock,"I got your message",18);
   
   if (n < 0) {
      perror("ERROR writing to socket");
      exit(1);
   }
	
}
