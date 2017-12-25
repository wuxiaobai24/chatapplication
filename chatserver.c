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

/* fifo path */
char register_file_path[BUFF_SZ];
char login_file_path[BUFF_SZ];
char sendmsg_file_path[BUFF_SZ];

/* fifo fd */
int reg_fifo_fd, login_fifo_fd, sendmsg_fifo_fd;

/* thread tid */
pthread_t register_tid, login_tid, sendmsg_tid;

/************************************************************************/
/* function declaration */

/* thread function */
void *register_func(void *arg);     /* the function for register */
void *login_func(void *arg);        /* the function for login */
void *chat_func(void *arg);         /* the function for chat */
void configuration();               /* read the configuration file and some value */
void pass1(int fd,int *lineStart);  /* get the pre line offset */
void pass2(int fd,int *lineStart);  /* parse the configuration pre line */
void parseLine(char *buffer,int readCount);
void become_daemon();               /* let the server become a daemon process */
void init_thread();                 /* init three listen thread */
void listen_loop();                 /* start to listen loop */
void fatalError(char *prompt);      /* a fatal error which server should exit */
void init();                        /* init the chat server */
void init_fifo();                   /* init the fifo fd */
void *login_thread_func(void *);    /* login thread function */
void *register_thread_func(void *); /* register thread function */
void *sendmsg_thread_func(void *);   /* sendmsg thread function */


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
    int res,fd;
    int lineStart[5]; /* the configuration file have only 4 line. */
    lineStart[0] = 0; 

    /* read the configuration file */
    /* the work dir is root now, so we should use the absolute path*/
    if ((fd = open(CONFIGURATIONFILE_PATH,O_RDONLY)) == -1)
            fatalError("configuration:open\n");
    /* parse configuration */
    /* the file pathname should less than 100 character */
    pass1(fd,lineStart);  //get pre line offset
    pass2(fd,lineStart); // parse the configuration

    if (res == -1) fatalError("configuration:read\n");
}

/*********************************************************************/
/* get pre line offset in fd*/

void pass1(int fd,int *lineStart) {
    int charRead,i;
    char buffer[BUFF_SZ];
    int fileOffset = 0;
    int lineCount = 1;
    while(TRUE) {
        charRead = read(fd,buffer,BUFF_SZ);
        if (charRead == 0) break;
        if (charRead == 1) fatalError("pass1");

        for(i = 0;i < charRead;i++) {
            fileOffset++;
            if (buffer[i] == '\n') lineStart[lineCount++] = fileOffset;
        }
        if (lineCount == 4) break; /* we just read the first four line */
    }
    if (lineCount != 4) lineStart[lineCount] = fileOffset;
}

/*******************************************************************/
/* prase the configuration pre line */

void pass2(int fd,int *lineStart) {
    int i, charRead;
    char buffer[BUFF_SZ]; // so pre line can not be longer than BUFF_SZ
    for(i = 0;i < 4;i++) {
        lseek(fd,lineStart[i],SEEK_SET);
        charRead = read(fd, buffer, lineStart[i+1] - lineStart[i]);
        buffer[charRead] = '\0';    //convenient to strcpy 
        parseLine(buffer,charRead);
    }
}

/***************************************************************/

void parseLine(char *buffer,int charRead) {
    if(charRead < 2) return;
    switch(buffer[0]) {
        case 'L':
            if(buffer[1] != ':') fatalError("parseLine:L");
            strcpy(login_file_path,buffer+2);
            break;
        case 'R':
            if(buffer[1] != ':') fatalError("parseLine:R");
            strcpy(register_file_path,buffer+2);
            break;
        case 'S':
            if(buffer[1] != ':') fatalError("parseLine:S");
            strcpy(sendmsg_file_path,buffer+2);
            break;
        case 'M':
            if(buffer[1] != ':') fatalError("parseLine:M");
            logined_users_max = atoi(buffer+2);
            break;
        default:
            fatalError("parseLine");
            break;
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
    
    /* init three fifo */
    init_fifo();

    /* init three thread */
    init_thread();
}

/*****************************************************************/
/* init the fifo fd */
void init_fifo() {
    int res;
    reg_fifo_fd = open(register_file_path, O_RDONLY);
    if (reg_fifo_fd == -1) fatalError("init_fifo:reg_fifo");

    login_fifo_fd = open(login_file_path, O_RDONLY);
    if (login_fifo_fd == -1) fatalError("init_fifo:login_fifo");

    sendmsg_fifo_fd = open(sendmsg_file_path, O_RDONLY);
    if (sendmsg_fifo_fd == -1) fatalError("init_fifo:sendmsg_fifo_fd");
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
