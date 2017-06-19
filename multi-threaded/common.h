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

