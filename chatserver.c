#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include "userlist.h"
#include "userfile.h"
#include "config.h"
#include "message.h"

#define BUFF_SZ 100
#define CONFIGURATIONFILE_PATH "/home/wukunhan2015170297/.chatapplication"
#define USERFILE_PATH "/home/wukunhan2015170297/chatapplication/data/userfile"

/* if define DEBUG, the server will not call the become_daemon */
#define DEBUG

#define TRUE 1
#define FALSE 0

/************************************************************************/
/* global value */
userfile_t userfile; /* user list */
userlist_t userlist; /* logined user list */
int logined_users_max = 1000;

/* server config */
config_t *config;

/* messager */
messenger_t *reg_reciver;
messenger_t *login_reciver;
messenger_t *msg_reciver; /* chat message reciver */


/* thread tid */
pthread_t register_tid, login_tid, sendmsg_tid;

/************************************************************************/
/* function declaration */

/* thread function */
void *register_func(void *arg);     /* the function for register */
void *login_func(void *arg);        /* the function for login */
void *chat_func(void *arg);         /* the function for chat */
void configuration();               /* read the configuration file and some value */
void become_daemon();               /* let the server become a daemon process */
void init_thread();                 /* init three listen thread */
void listen_loop();                 /* start to listen loop */
void fatalError(char *prompt);      /* a fatal error which server should exit */
void init();                        /* init the chat server */
void init_recivers();              /* init messenger */
void *login_thread_func(void *);    /* login thread function */
void *register_thread_func(void *); /* register thread function */
void *sendmsg_thread_func(void *);   /* sendmsg thread function */
void cleaner();                     /* remove the fifo if signaled */

/***********************************************************************/
/* main function */
int main (int argc,char **argv) {
#ifndef DEBUG
    become_daemon();
#endif
    configuration();
    init();
    listen_loop();
    return 0;
}

/**********************************************************************/
/* let server become a daemon process */
void become_daemon() {
    pid_t pid;
    int fd;
    if ((pid = fork()) < 0) fatalError("become_daemon:fork\n");
    else if (pid > 0) exit(0); /* the praent process should exit */

    /* child process */
    /* creates a new session */
    if (setsid() == -1) fatalError("become_daemon:setsid\n");
    
    /* change the work dir to the root dir */
    if (chdir("/")  != 0) fatalError("become_daemon:chadir\n");
    
    /* open the null dev */
    if ((fd = open("/dev/null",O_RDWR,0)) == -1) {
        /* open failure */
        fatalError("become_daemon:open null dev\n");
    }
    /* redirect standard output,input and error to null dev */
    if (dup2(fd,STDIN_FILENO) == -1 ||
            dup2(fd,STDOUT_FILENO) == -1 ||
            dup2(fd,STDERR_FILENO) == -1) {
        fatalError("become_daemon:redurect\n");
    }
    if(close(fd) == -1) fatalError("become_daemon:close_null_dev\n");

    /* set file mode creation mask */
    umask(0027);
}

/**********************************************************************/
/* read configuration file and parse configuration */

void configuration() {
    int res;
    config  = (config_t *)malloc(sizeof(config_t));
    if (config == NULL) fatalError("malloc config failure");

    res = config_parse(CONFIGURATIONFILE_PATH,config);

    printf("register_file_path is %s\n",config->reg_path);
    printf("login_file_path is %s\n",config->login_path);
    printf("sendmsg_file_path is %s\n",config->sendmsg_path);
    printf("logined_users_max is %d\n",config->max_user);
    if (res != 0) {
        fprintf(stderr,"config_parse error\n");
        exit(-1);
    }
}


/*******************************************************************/
void fatalError(char *prompt) {
    perror(prompt);
    exit(-1);
}


/******************************************************************/
/* init */
void init() {
    int res;
    /* init the userlist */
    res = userlist_init(&userlist,logined_users_max);
    if (res != SUCCESS) fatalError("userlist_init");

    /* init the userfile */
    res = userfile_init(&userfile,USERFILE_PATH);
    if (res != USERFILE_SUCCESS) fatalError("userfile_init");
    
    /* init three messenger */
    init_recivers();

    
    /* init three thread */
    init_thread();
}

/*****************************************************************/
/* init reciver */
void init_recivers() {
    int res;
    //create fifo
    res =  mkfifo(config->reg_path,0777);
    if (res != 0) fatalError("init_fifo:create reg_fifo");

    res = mkfifo(config->login_path,0777);
    if (res != 0) fatalError("init_fifo:create login_file");

    res = mkfifo(config->sendmsg_path,0777);
    if (res != 0) fatalError("init_fifo:create sendmsg_file");


    /* handle some signals */
    signal(SIGKILL, cleaner);
    signal(SIGINT, cleaner);
    signal(SIGTERM, cleaner);


    //malloc messenger 
    reg_reciver = (messenger_t *)malloc(sizeof(messenger_t));
    if (reg_reciver == NULL) fatalError("init_reciver:malloc reg_reciver");

    login_reciver = (messenger_t *)malloc(sizeof(messenger_t));
    if (login_reciver == NULL) fatalError("init_reciver:malloc login_reciver");

    msg_reciver = (messenger_t *)malloc(sizeof(messenger_t));
    if (msg_reciver == NULL) fatalError("init_reciver:malloc msg_reciver");

    //init messenger
    if (messenger_init(reg_reciver,config->reg_path,Reciver) != 0)
        fatalError("init_reciver:messenger_init reg_reciver");
    if (messenger_init(login_reciver,config->login_path,Reciver) != 0)
        fatalError("init_reciver:messenger_init login_reciver");
    if (messenger_init(msg_reciver,config->sendmsg_path,Reciver) != 0)
        fatalError("init_reciver:messenger_init msg_reciver");
}
/****************************************************************/
/*init thread */

void init_thread() {
   int res;
    /* create register thread */
   res = pthread_create(&register_tid,NULL,register_thread_func,NULL);
   if (res == -1) fatalError("init_thread:register");

   /* create login thread */
   res = pthread_create(&login_tid,NULL,login_thread_func,NULL);
   if (res == -1) fatalError("init_thread:login");

   res = pthread_create(&sendmsg_tid,NULL,sendmsg_thread_func,NULL);
   if (res == -1) fatalError("init_thread:sendmsg");

}

/******************************************************************/
void listen_loop() {
   pthread_join(register_tid,NULL);
   pthread_join(login_tid,NULL);
   pthread_join(sendmsg_tid,NULL);
}

/******************************************************************/
/* register thread function */

void *register_thread_func(void *arg) {
    printf("register_thread_func\n");
}

/******************************************************************/
/* login_thread_function */

void *login_thread_func(void *arg) {
    printf("login_thread\n");

}

/*****************************************************************/
/* sendmsg_thread_func */

void *sendmsg_thread_func(void *arg) {
    printf("register_thread\n");
}



/******************************************************************/
/* remove the fifo if server down */
void cleaner() {
    unlink(config->reg_path);
    unlink(config->login_path);
    unlink(config->sendmsg_path);
}
