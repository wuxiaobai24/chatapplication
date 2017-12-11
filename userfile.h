#ifndef _USER_FILE
#define _USER_FILE

#include "user.h"

/* init usefile */
int userfile_init(char *user_filename);
/* add a user in userfile */
int userfile_add_user(user_t *user);
/* search a user form userfile by username */
int search_user(char *username,user_t *user);
/* remove a user form userfile */
int remove_user(char *username);
/* userfile_destroy */
int userfile_destroy();


#endif
