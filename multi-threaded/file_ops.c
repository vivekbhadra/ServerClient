#include "common.h"

int initialize_database(server_data_t *servData, const char *db_name)
{
	if (!servData) {
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
	} else {
	    while(fgets(servData->message, BUF_LEN, servData->fp) != EOF) {
            return servData->message;
	    }
	}
}

char * write_database(server_data_t *servData)
{
	if (!servData) {
	    return -1;
	} else {
	    fputs (servData->message,servData->fp);
	}
}
