#ifndef COMMON_H
#define COMMON_H
#endif

<<<<<<< HEAD
=======
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <error.h>

>>>>>>> ed975e7d44293936fb248b91d6b7672db7ebe28a
#define MESSAGE_LEN 2000

typedef enum request {
	READ_REQ,
	WRITE_REQ
}request_t;

typedef struct message {
	request_t req;
    char message[MESSAGE_LEN];
}message_t;

<<<<<<< HEAD
typedef struct server_data {
    char message[BUF_LEN];
    pthread_mutex_t rwsem;
    FILE *fp;
}server_data_t;

extern int initialize_database(server_data_t *servData, const char *db_name);
extern char * read_database(server_data_t *servData, const char *db_name);
extern void write_database(void);
=======
>>>>>>> ed975e7d44293936fb248b91d6b7672db7ebe28a
