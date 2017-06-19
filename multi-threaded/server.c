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

typedef struct server_data {
    char message[BUF_SIZE];
    pthread_mutex_t rwsem;
    FILE *fp;
}server_data_t;

static server_data_t servData;

struct threadData {
    struct sockaddr_in *cl_addr;
    int sock;
    int len;
    char clientAddr[CLADDR_LEN];
    server_data_t *db;        
};

int initialize_database(server_data_t *servData, const char *db_name)
{
    if (!servData) {
        return -1;
    } else {
        pthread_mutex_init(&servData->rwsem, NULL);
        servData->fp = fopen(db_name,"r");
        if (!servData->fp) {
            fprintf(stderr, "ERROR couldn't open database file \n");
            perror("");
            return -1;
        }
    }
    return 0;
}

int read_from_database(char *buff, server_data_t *servData)
{
    char buffer[2000];
    if (!servData) {
        return -1;
    } else {
        pthread_mutex_lock(&servData->rwsem);
        if(fgets(buff, MESSAGE_LEN, servData->fp) == NULL)
            fprintf(stderr, "ERROR reading file\n");
        //fgets(buffer, MESSAGE_LEN, servData->fp);
        //while(fgets(buffer, MESSAGE_LEN - 1, servData->fp) != NULL) 
        //{
            //printf("Hello Vivek\n");
            //printf ("%s\n", buffer); /* ...such as show it on the screen */
        //}  
        pthread_mutex_unlock(&servData->rwsem);
    }
    
    return 0;
}

int write_database(server_data_t *servData, const char * buffer)
{
    if (!servData) {
        return -1;
    } else {
        pthread_mutex_lock(&servData->rwsem);
        fputs (buffer,servData->fp);
        pthread_mutex_unlock(&servData->rwsem);
    }
}


/*
 * Per connection thread function for handling connection.
 * */
void *connection_handler(void *arg)
{
    char buffer[BUF_SIZE];
    int ret;
    message_t *message;
    struct threadData *tData = (struct threadData *)arg;

    for (;;) {
        memset(buffer, 0, BUF_SIZE);
        ret = recvfrom(tData->sock,
	    		buffer,
			BUF_SIZE,
			0,
			(struct sockaddr *) tData->cl_addr,
			&tData->len);
	if(ret < 0) {
	    fprintf(stderr, "Error receiving data: %s\n", gai_strerror(ret));
	    exit(EXIT_FAILURE);
	}
	message = (message_t *)buffer;
	fprintf(stdout, "Received %s request from %s\n",
			message->req ? "WRITE" : "READ",
			tData->clientAddr);

	switch(message->req) {
	case READ_REQ:
            {
            char buff[2000];
            if(read_from_database(buff, tData->db) < 0) {
                fprintf(stderr, "Error reading from database \n");
                exit(EXIT_FAILURE);      
            } else {
                strcpy(message->message, buff);
                fprintf(stdout, "message being sent %s\n", message->message);
            }
	    ret = sendto(tData->sock, 
                        message->message, 
                        BUF_SIZE, 
                        0, 
                        (struct sockaddr *) tData->cl_addr, 
                        tData->len);
            if (ret < 0) {
	        fprintf(stderr, "Error sending data!\n");
	    	exit(EXIT_FAILURE);
	    }
	    fprintf(stdout, "Sent data to %s: %s\n", tData->clientAddr, message->message);
	    break;
            }
        case WRITE_REQ:
            if(write_database(tData->db, message->message) < 0) {
                fprintf(stderr, "Error writing to database \n");
                exit(EXIT_FAILURE);      
            }
            break;
	}
    }
	return 0;
}

int main(int argc, char**argv) {
    struct sockaddr_in addr, cl_addr;
    int sockfd, len, ret, newsockfd;
    char buffer[BUF_SIZE];
    pid_t childpid;
    char clientAddr[CLADDR_LEN];
    pthread_t thread_id;
    int client_sock, c;
    struct threadData *threadParams;
    char *db_name[20];

    if (argc < 2) {
        printf("usage: server < database file name >\n");
        exit(1);
    }

    if (initialize_database(&servData, argv[1]) < 0) {
        fprintf(stderr, "Error coudln't initialize databse\n");
        exit(EXIT_FAILURE);
    } 

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
            threadParams->db = &servData;
        }

        if(pthread_create( &thread_id, NULL,  connection_handler, (void*) threadParams) < 0) {
            fprintf(stderr, "Error Client handler function couldn't be crated.\n");
            exit(EXIT_FAILURE);
        }
     }
     exit(EXIT_SUCCESS);
}
