#ifndef _USER_T
#define _USER_T

#define MAX_USERNAME_LEN 100
#define MAX_PASSWD_LEN 100

typedef struct {
    char username[MAX_USERNAME_LEN + 1];
    char passwd[MAX_PASSWD_LEN + 1];
} user_t;

#endif
