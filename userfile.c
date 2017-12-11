#include "userfile.h"
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/********************************************************/
/* static value */
static pthread_mutex_t mutex;
static 

/********************************************************/
/* init userfile */
int userfile_init(char *user_filename) {
    userfile_fd = open(filename,O_RDWR | O_CREAT | O_APPEND);
    if (userfile_fd == -1) {
        perror("Fail to open the userfile %s\n",user_filename);
        exit(/* EXITFAILURE */ 1);
    }
    pthread_mutex_init(&mutex);
}

/*******************************************************/
/* destroy userfile (just close userfile, not remove the file */
int userfile_destroy() {
    if (close(userfile_fd) == -1) return -1; /* close file failure */

    if (pthread_mutex_destroy(&mutex) != 0) 
        return 1;/* destroy mutex failure */

    userfile_fd = -1;
}

/*****************************************************/
/* add a user to userfile */
int userfile_add(user_t *user) {
    if (userfile_fd == -1) return -1;
    if (user == NULL) return -2;
        
    return 0;
}


/******************************************************/
/* search a user form userfile by username */

int search_user(char *username,user_t *user) 
    
}
