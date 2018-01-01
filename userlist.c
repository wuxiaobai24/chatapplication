#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "userlist.h"
#include "user.h"

/*********************************************************************************/

static void fatalError(char *prompt) {
    fprintf(stderr,"FatalError: %s\n",prompt);
    exit(/* EXITFAILURE*/1);
}

/*********************************************************************************/
/* Init the userlist */

int userlist_init(userlist_t *userlist,int max_users) {
    int res;
    if (userlist == NULL) return POINTER_NULL;
    
    /* malloc users memory */
    userlist->users = (user_t*)malloc(sizeof(user_t)*max_users);
    if (userlist->users == NULL) fatalError("create_userlist:ERROF: malloc fail\n");

    userlist->max_users = max_users; /* set max_users */
    userlist->user_len = 0; /* set the user_len */

    /* init mutex */
    res = pthread_mutex_init(&userlist->mutex,NULL);
    if (res != 0) return res;

    return SUCCESS;
}

/**************************************************************/
/* detsory the userlist */

int destory_userlist(userlist_t *userlist) {
    int res;
    if (userlist == NULL) return POINTER_NULL;
    
    if (userlist->users == NULL) return POINTER_NULL;
    free(userlist);

    res = pthread_mutex_destroy(&userlist->mutex);
    return SUCCESS;
}

/*************************************************************/
/* get user from userlist by index */

int userlist_get(userlist_t *userlist,user_t *user,int index) {
    if (userlist == NULL || user == NULL) return POINTER_NULL;
    if (index > userlist->max_users) return INDEX_ERROR;
    
    pthread_mutex_lock(&userlist->mutex);
    //memcopy(&(userlist->users[index]),user,sizeof(user_t));
    if (index > userlist->user_len) {
        pthread_mutex_unlock(&userlist->mutex);
        return INDEX_ERROR;
    }
    *user = userlist->users[index];
    pthread_mutex_unlock(&userlist->mutex);

    return SUCCESS;
}

/**************************************************************/
/* set user information */

int userlist_set(userlist_t *userlist,user_t *user,int index) {
    if (userlist == NULL || user == NULL) return POINTER_NULL;
    if (index > userlist->max_users) return INDEX_ERROR;
    
    pthread_mutex_lock(&userlist->mutex);
    //memcopy(&(userlist->users[index]),user,sizeof(user_t));
    if (index > userlist->user_len) {
        pthread_mutex_unlock(&userlist->mutex);
        return INDEX_ERROR;
    }
    userlist->users[index] = *user;
    pthread_mutex_unlock(&userlist->mutex);

    return SUCCESS;
}

/*************************************************************/
/* add a user to userlist */

int userlist_add(userlist_t *userlist,user_t *user) {
    if (userlist == NULL || user == NULL) return POINTER_NULL;
    
    pthread_mutex_lock(&userlist->mutex);
    if (userlist->user_len == userlist->max_users) {
        pthread_mutex_unlock(&userlist->mutex);
        return USER_TO_MUCH;
    }
    userlist->users[userlist->user_len++] = *user;
    pthread_mutex_unlock(&userlist->mutex);

    return SUCCESS; 
}

/************************************************************/
/* search a user in userlist 
 * Return :index */

int userlist_search(userlist_t *userlist,user_t *user) {
    int i;
    if (userlist == NULL || user == NULL) return POINTER_NULL;
    
    pthread_mutex_lock(&userlist->mutex);
    for(i = 0;i < userlist->user_len;i++) {
        /* ths name is unique iddentification */
        if (strcmp(userlist->users[i].username,user->username) == 0)
            break;
    }
    if (i == userlist->user_len) i = NOT_FOUND;
    pthread_mutex_unlock(&userlist->mutex);

    return i;
}

/***************************************************************/
/* remover a user form userlist */

int userlist_remove(userlist_t *userlist,int index) {
    int i;
    if (userlist == NULL) return POINTER_NULL;
    if (index > userlist->max_users) return INDEX_ERROR; 

    pthread_mutex_lock(&userlist->mutex);

    userlist->user_len--;
    for(i = index;i < userlist->user_len;i++)
        userlist->users[i] = userlist->users[i+1];

    pthread_mutex_unlock(&userlist->mutex);
    return SUCCESS;
}
