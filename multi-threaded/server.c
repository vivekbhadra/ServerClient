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
#define MAX_BACKLOG 5
#define MAX_CLIENT 1000

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
    int connection_id;
};

int initialize_database(server_data_t *servData, const char *db_name)
{
    if (!servData) {
        return -1;
    } else {
        pthread_mutex_init(&servData->rwsem, NULL);
        servData->fp = fopen(db_name,"w+");
        if (!servData->fp) {
            fprintf(stderr, "ERROR couldn't open database file \n");
            perror("");
            return -1;
        }
    }
    return 0;
}

int read_from_database(server_data_t *servData)
{
    if (!servData) {
        return -1;
    } else {
        pthread_mutex_lock(&servData->rwsem);
        fseek( servData->fp, 0, SEEK_SET );
        if(fgets(servData->message, MESSAGE_LEN, servData->fp) == NULL) {
            fprintf(stderr, "ERROR reading file\n");
        }
        fflush(servData->fp);
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
        fseek(servData->fp, 0, SEEK_SET);
        fputs (buffer,servData->fp);
        fflush(servData->fp);
        pthread_mutex_unlock(&servData->rwsem);
    }

    return 0;
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
	        exit(EXIT_FAILURE);;
	    } else if (ret == 0) {
	    	fprintf(stderr, "Client [connection id %d] closed connection\n", tData->connection_id);
	    	pthread_exit(NULL);
	    }
	    message = (message_t *)buffer;
	    fprintf(stdout, "<== Received %s request from %s\n",
		    message->req ? "WRITE" : "READ",
		    tData->clientAddr);

	    switch(message->req) {
        case READ_REQ:
            if(read_from_database(tData->db) < 0) {
                fprintf(stderr, "Error reading from database \n");
                exit(EXIT_FAILURE);
            }

            ret = sendto(tData->sock,
	    		tData->db->message,
				BUF_SIZE,
				0,
				(struct sockaddr *) tData->cl_addr,
				tData->len);
            if (ret < 0) {
	            fprintf(stderr, "Error sending data!\n");
	    	    exit(EXIT_FAILURE);
	        }
            fprintf(stdout, "SEND message to client ==> %s\n", tData->db->message);
	        break;

        case WRITE_REQ:
            if(write_database(tData->db, message->message) < 0) {
                fprintf(stderr, "Error writing to database \n");
                exit(EXIT_FAILURE);
            } else {
          	    fprintf(stderr, "UPDATED message from client.\n");
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
    pthread_t thread_id[MAX_CLIENT];
    int client_count = 0;
    int client_sock, c;
    struct threadData *threadParams[MAX_CLIENT];
    char *db_name[20];
    int i;

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
    listen(sockfd, MAX_BACKLOG);

    for (;;) {
        len = sizeof(cl_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cl_addr, &len);
        if (newsockfd < 0) {
            fprintf(stderr, "Error accepting connection: %s\n", gai_strerror(ret));
	    exit(EXIT_FAILURE);
        }
        fprintf(stdout, "Connection [%d] accepted...\n", client_count);
        inet_ntop(AF_INET, &(cl_addr.sin_addr), clientAddr, CLADDR_LEN);
        /*
        * Create a new child process to handle the new client.
        * */
        threadParams[client_count] = malloc(sizeof(struct threadData));
        if (!threadParams[client_count]) {
            fprintf(stderr, "Error accepting connection: %s\n", gai_strerror(ret));
  	        exit(EXIT_FAILURE);
        } else {
            threadParams[client_count]->cl_addr = (struct sockaddr_in *) &cl_addr;
            threadParams[client_count]->sock = newsockfd;
            threadParams[client_count]->len = len;
            strncpy(threadParams[client_count]->clientAddr, clientAddr, CLADDR_LEN);
            threadParams[client_count]->db = &servData;
            threadParams[client_count]->connection_id = client_count;
        }
        if(pthread_create( &thread_id[client_count],
        		NULL,
				connection_handler,
				(void*) threadParams[client_count]) < 0) {
            fprintf(stderr, "Error Client handler function couldn't be created.\n");
        }
        client_count++;
        if (client_count > MAX_CLIENT)
        	break;
     }
     for(i = 0; i < client_count; i++) {
    	 pthread_join(thread_id[i], NULL);
         free(threadParams[i]);
     }
     exit(EXIT_SUCCESS);
}
