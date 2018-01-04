#ifndef _USER_FILE_H
#define _USER_FILE_H
#include "user.h"
#include <pthread.h>
#include <sys/types.h>
#include <fcntl.h>

#define USERFILE_NO_USER 1
#define USERFILE_ERROR -1
#define USERFILE_SUCCESS 0

typedef struct {
    int fd;
    pthread_mutex_t mutex;
} userfile_t;

/* Init the userfile by the file path */
int userfile_init(userfile_t *userfile, char *path);

/* add a user to userfile */
int userfile_add_user(userfile_t *userfile, user_t *user);

/* search a user in userfile by username */
int userfile_search_user(userfile_t *userfile, user_t *user);

#endif
