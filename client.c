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
#include "message.h"

/* some macro definition */
#define BUFF_SZ 100

/****************************************************************************/
/* global val */

/* login user information */
user_t *login_user;
messenger_t *user_reciver;

/* server messenger */
messenger_t *reg_sender;
messenger_t *login_sender;
messenger_t *msg_sender; /* chat messeng */


config_t *config;

/****************************************************************************/
/* function declaration */
int main(int argc,char **argv);         /* the main function */
int init();                             /* init the client */
void fatalError(char *prompt);          /* fatal error */
void listen_loop();                     /*listen loop, wait user input*/
void register_user();                   /* register */
void login();                           /* login user */
void sendmsg();                         /* send msg */
void init_sender();                     /* init the messenger for server */



/**************************************************************************/
/* main function */

int main(int argc,char **argv) {
    init();
    listen_loop();
    return 0;
}


/*************************************************************************/
/* fatal error, clien will termina */
void fatalError(char *prompt) {
    perror(prompt);
    exit(-1/* Failure */);
}

/*************************************************************************/
/* init the client*/

void init() {
    /* init the configuration */
    init_config();

    /* init fifo */
    init_fifo();

}


/*************************************************************************/
/* init the config */
void init_config() {
    int res;
    config = (config_t *)malloc(sizeof(config_t));
    
    if (config == NULL) fatalError("init_config:malloc config failure");

    res = parse_config(CONIGURATIONFILE_PATH,config);

    if (res != 0) fatalError("init_config:parse_cofnig");
}

/**************************************************************************/
/* init the messenger for server */
void init_sender() {
    int res;

    reg_sender = (messenger_t*)malloc(sizeof(messenger_t));
    login_sender = (messenger_t*)malloc(sizeof(messenger_t));
    msg_sender = (messenger_t*)malloc(sizeof(messenger_t));

    if (reg_sender == NULL || login_sender == NULL || msg_sender == NULL)
        fatalError("init_sender:malloc failure");

    res = messenger_init(reg_sender,Sender);
    if (res != 0) fatalError("init_sender:reg_sender init failure");

    res = messenger_init(login_sender,Sender);
    if (res != 0) fatalError("init_sender:login_sender init failure");

    res = messenger_init(msg_sender,Sender);
    if (res != 0) fatalError("init_sender:msg_sender init failure");
}

/******************************************************************************/
/* listen loop  */

void listen_loop() {
    int input;
    while(1) {
        /*if user logined, show user information, check user_reciver */
        if (login_user != NULL) {
            printf("User:%s\n",login_user->username);
            reciver_msg();
        }
        show_prompt();
        input = reciver
    }
}
