<<<<<<< HEAD
#ifndef COMMON_H
#define COMMON_H
#endif
=======
#include "common.h"
>>>>>>> ed975e7d44293936fb248b91d6b7672db7ebe28a

int initialize_database(server_data_t *servData, const char *db_name)
{
	if (!servData) {
<<<<<<< HEAD
		return -EINVAL;
	} else {
	    pthread_mutex_init(&servData->rwsem, NULL);
        servData->fp = fopen(db_name,"w+");

        if (!servData->fp) {
   	        fprintf(stderr, "ERROR couldn't open database file %s\n", gai_strerror(fp));
            return -EIO;
        }
	}
	return 0;
}

char * read_database(server_data_t *servData)
{
	if (!servData) {
		return -EINVAL;
=======
		return -1;
	} else {
	    pthread_mutex_init(&servData->rwsem, NULL);
            servData->fp = fopen(db_name,"w+");

            if (!servData->fp) {
   	        fprintf(stderr, "ERROR couldn't open database file \n");
                return -1;
            }
        }
	return 0;
}

char * read_from_database(server_data_t *servData)
{
	if (!servData) {
		return -1;
>>>>>>> ed975e7d44293936fb248b91d6b7672db7ebe28a
	} else {
	    while(fgets(servData->message, BUF_LEN, servData->fp) != EOF) {
            return servData->message;
	    }
	}
}

char * write_database(server_data_t *servData)
{
	if (!servData) {
<<<<<<< HEAD
	    return -EINVAL;
=======
	    return -1;
>>>>>>> ed975e7d44293936fb248b91d6b7672db7ebe28a
	} else {
	    fputs (servData->message,servData->fp);
	}
}
