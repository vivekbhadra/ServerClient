#ifndef COMMON_H
#define COMMON_H
#endif

#define MESSAGE_LEN 2000

typedef enum request {
	READ_REQ,
	WRITE_REQ
}request_t;

typedef struct message {
	request_t req;
    char message[MESSAGE_LEN];
}message_t;

typedef struct server_data {
    char message[BUF_LEN];
    pthread_mutex_t rwsem;
    FILE *fp;
}server_data_t;

extern int initialize_database(server_data_t *servData, const char *db_name);
extern char * read_database(server_data_t *servData, const char *db_name);
extern void write_database(void);
