#ifndef COMMON_H
#define COMMON_H
#endif

int initialize_database(server_data_t *servData, const char *db_name)
{
	if (!servData) {
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
	} else {
	    while(fgets(servData->message, BUF_LEN, servData->fp) != EOF) {
            return servData->message;
	    }
	}
}

char * write_database(server_data_t *servData)
{
	if (!servData) {
	    return -EINVAL;
	} else {
	    fputs (servData->message,servData->fp);
	}
}
