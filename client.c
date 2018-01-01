#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "user.h"
#include "config.h"

/* some macro definition */
#define BUFF_SZ 100

/****************************************************************************/
/* global val */

/* login user information */
user_t *login_user;

int reg_fifo_fd, login_fifo_fd, sendmsg_fifo_fd;
int clien_fifo_fd;

config_t *config;

/****************************************************************************/
/* function declaration */
int main(int argc,char **argv);         /* the main function */
int init();                             /* init the client */
void fatalError();                      /* fatal error */
void listen_loop();                     /*listen loop, wait user input*/
void register_user();                   /* register */
void login();                           
