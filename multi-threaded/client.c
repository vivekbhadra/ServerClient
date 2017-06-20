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
    struct sockaddr_in addr;
    int sockfd, ret;
    char buffer[BUF_SIZE];
    char * serverAddr;
    message_t req;
    char choice[32], *endptr;
    char temp_msg[BUF_SIZE] = {0};

    if (argc < 2) {
        fprintf(stdout, "usage: client < ip address >\n");
        exit(1);
    }

    serverAddr = argv[1];
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "Error creating socket!\n");
        exit(1);
    }

    fprintf(stdout, "Socket created...\n");
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(serverAddr);
    addr.sin_port = PORT;

    ret = connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));
    if (ret < 0) {
        fprintf(stdout, "Error connecting to the server!\n");
        exit(1);
    }
    fprintf(stdout, "Connected to the server...\n");
    memset(&req, 0, sizeof(message_t));

    while (1) {
        fprintf(stdout, "Enter your request(read=0 write=1): ");
        fgets(choice, 32, stdin);
        req.req = strtol(choice, &endptr, 10);
        switch(req.req) {
        case READ_REQ:
            fprintf(stdout, "Sending the READ request now \n");
            ret = sendto(sockfd, 
                        (const void *)&req, 
                        BUF_SIZE, 
                        0, 
                        (struct sockaddr *) &addr, 
                        sizeof(addr));
            if (ret < 0) {
                fprintf(stderr, "Error sending the %s request\n", 
                        req.req ? "WRITE" : "READ");
            } else {
                fprintf(stdout, "Waiting for the server response...\n");
                ret = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);
                if (ret < 0) {
                    fprintf(stderr, "Error receiving data!\n");
                } else {
                    fprintf(stdout, "==> Received: ");
                    fputs(buffer, stdout);
                    fprintf(stdout, "\n");
                }
            }
            break;
        case WRITE_REQ:
            fprintf(stdout, "Enter your message: ");
            fgets(temp_msg, 2000, stdin);
            strncpy(req.message, temp_msg, BUF_SIZE);
            fprintf(stderr, "Message %s \n", temp_msg);
            ret = sendto(sockfd, (const void *)&req,
                         BUF_SIZE,
                         0,
                         (struct sockaddr *) &addr,
                         sizeof(addr));
            if (ret < 0) {
                fprintf(stderr, "Error sending %s request!\n", 
                        req.req ? "WRITE" : "READ");
            } else {
                fprintf(stdout, "<== Sent %s request with new message: %s\n",
                        req.req ? "WRITE" : "READ",
                        req.message);
                fprintf(stderr, "\n");
            }
            break;
        default:
            fprintf(stdout, "Unknown request\n");
        }
    }
    exit(EXIT_SUCCESS);
}
