#include"stdio.h"
#include"stdlib.h"
#include"sys/types.h"
#include"sys/socket.h"
#include"string.h"
#include"netinet/in.h"
#include"netdb.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"

#define PORT 4444
#define BUF_SIZE 2000

int main(int argc, char**argv) {
    struct sockaddr_in addr, cl_addr;
    int sockfd, ret;
    char buffer[BUF_SIZE];
    struct hostent * server;
    char * serverAddr;
    message_t req;
    char choice;

    if (argc < 2) {
        printf("usage: client < ip address >\n");
        exit(1);
    }

    serverAddr = argv[1];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Error creating socket!\n");
        exit(1);
    }
    printf("Socket created...\n");

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(serverAddr);
    addr.sin_port = PORT;

    ret = connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));
    if (ret < 0) {
        printf("Error connecting to the server!\n");
        exit(1);
    }
    printf("Connected to the server...\n");

    memset(&req, 0, sizeof(message_t));

    while (1) {
    	printf("Enter your request(read=0 write=1): ");
        scanf("%u", &req.req);
        printf("req.req: %u\n", req.req);
        switch(req.req) {
        case READ_REQ:
            fprintf(stdout, "Sending the READ request now \n");
            ret = sendto(sockfd, (const void *)&req, BUF_SIZE, 0, (struct sockaddr *) &addr, sizeof(addr));
            if (ret < 0) {
            	fprintf(stderr, "Error sending the %s request\n", req.req ? "WRITE" : "READ");
            } else {
            	fprintf(stdout, "Waiting for the server response...");
            	ret = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);
            	if (ret < 0) {
            	    printf("Error receiving data!\n");
            	} else {
            	    printf("Received: ");
            	    fputs(buffer, stdout);
            	    printf("\n");
            	}
            }
            break;
        case WRITE_REQ:
            fprintf(stdout, "Enter your message: ");
            fgets(req.message, BUF_SIZE, stdin);
                ret = sendto(sockfd, (const void *)&req,
                		BUF_SIZE,
						0,
						(struct sockaddr *) &addr,
						sizeof(addr));
                if (ret < 0) {
                	fprintf(stderr, "Error sending %s request!\n", req.req ? "WRITE" : "READ");
                } else {
                    fprintf(stdout, "Sent %s request with new message %s\n",
            		            req.req ? "WRITE" : "READ",
            			    	req.message);
                    fputs(req.message, stdout);
                    fprintf(stderr, "\n");
                }
            break;
        default:
        	fprintf(stdout, "Unknown request\n");
        }
    }

    exit(EXIT_SUCCESS);
}