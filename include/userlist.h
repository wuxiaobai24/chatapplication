#ifndef USER_LIST_
#define USER_LIST_

#include <pthread.h>
#include "user.h"

/******************************************************************/
/* RETURN_FLAG */
#define SUCCESS 0
#define POINTER_NULL -1
#define INDEX_ERROR -2
#define USER_TO_MUCH -3
#define NOT_FOUND -4


typedef struct {
    user_t *users; /* the user_t data */
    int max_users; /* the userlist only contains max_users user.*/
    int user_len; /* the count of actual user in userlist */
    pthread_mutex_t mutex;
} userlist_t; /* a thread safe user_t container.*/

/* initial a user list which contain max_count user */
int userlist_init(userlist_t *userlist,int max_users);
/* destroy userlist */
int userlist_destory(userlist_t *userlist);
/* get a user from userlist by index */
int userlist_get(userlist_t *userlist,user_t *user,int index);
/* set a user in userlist by index */
int userlist_set(userlist_t *userlist,user_t *user,int index);
/* add user to userlist */
int userlist_add(userlist_t *userlist,user_t *user);
/* search user. */
int userlist_search(userlist_t *userlist,user_t *user);
/* remove a user */
int userlist_remove(userlist_t *userlist,int index);

#endif
