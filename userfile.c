#include "userfile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
/* '/n/n' is separator for users
 * '/n' is separator for username and passwd */

/*************************************************************************/
/* Init the userfile by file path *
 * Return: if init userfile success,return 0,else return -1 */

int userfile_init(userfile_t *userfile,char *path) {

    userfile->fd = open(path,O_CREAT | O_RDWR, 0600);
    if (userfile->fd == -1) return USERFILE_ERROR;
    
    if(pthread_mutex_init(&userfile->mutex,NULL)) return USERFILE_ERROR;
    return USERFILE_SUCCESS;
}

/* add a user to userfile */
int userfile_add_user(userfile_t *userfile, user_t *user) {
    if (userfile == NULL || user == NULL) return USERFILE_ERROR;
    pthread_mutex_lock(&userfile->mutex);
    
    if (lseek(userfile->fd,0,SEEK_END) == -1) return USERFILE_ERROR;

    write(userfile->fd, (void *)user, sizeof(user_t));

    pthread_mutex_unlock(&userfile->mutex);
    return USERFILE_SUCCESS;
}

/* search a user in userfile by username */
int userfile_search_user(userfile_t *userfile, user_t *user) {
    if (userfile == NULL || user == NULL) return USERFILE_ERROR;
    printf("userfile_search_user\n");
    user_t buf;
    int res;
    pthread_mutex_lock(&userfile->mutex);
    
    res = lseek(userfile->fd,0,SEEK_SET);
    printf("lseek res is %d\n",res);
    if (res == -1)  return USERFILE_ERROR;

    //search
    while(read(userfile->fd,(void *)&buf,sizeof(user_t)) ) {
       if (strcmp(user->username,buf.username) == 0) {
            pthread_mutex_unlock(&userfile->mutex);
            strcpy(user->passwd,buf.passwd);
            return USERFILE_SUCCESS;
        }
    }

    pthread_mutex_unlock(&userfile->mutex);
    return USERFILE_NO_USER;
}

