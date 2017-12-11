#ifndef _USER_T
#define _USER_T

#define MAX_NAME_LENGTH 100
#define MAX_PASSWD_LENGTH 100

typedef struct {
    char username[MAX_NAME_LENGTH];
    char passwd[MAX_PASSWD_LENGHTH];
} user_t;

#endif
