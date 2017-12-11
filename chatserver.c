#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include "userlist.h"

#define BUFF_SZ 100
#define CONFIGURATIONFILE_PATH "/home/wukunhan2015170297/.chatapplication"
/* if define DEBUG, the server will not call the become_daemon */
#define DEGUG

/************************************************************************/
/* global value */
int reg_fifo_fd, login_fifo_fd, chat_fifo_fd; /* fifo fd */
userlist_t userlist; /* user list */
userlist_t logined_userlist; /* logined user list */

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

/***********************************************************************/
/* main function */
int main (int argc,char **argv) {
#ifndef DEBUG
    become_daemon();
#endif
    configuration();
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

void configuration() {
    int i;
    char buf[BUFF_SZ];
    FILE *stream;
    size_t len = 0; ssize_t res;
    /* read the configuration file */
    /* the work dir is root now, so we should use the absolute path*/
    stream = fopen(CONFIGURATIONFILE_PATH,"r");
    if (stream == NULL) fatalError("configuration:fopen\n");
    
    /* parse configuration */

}

void handler(int sig) {
	unlink(REG_FIFO_NAME);
	unlink(LOGIN_FIFO_NAME);
	unlink(CHAT_FIFO_NAME);
    int i = 0;
    char buf[100];
    while(i < user_end) {
        sprintf(buf,"%s%s",CLIENT_PREFIX,users[i].username);
        unlink(buf);
        i++;
    }
	exit(1);
}

int find_user(USER *user_ptr);
int check_passwd(USER *user_ptr,int user_id);

void init_server_fifo(const char *fifo_name) {
	int res;
	if (access(fifo_name,F_OK) == -1) {
		res = mkfifo(fifo_name,0777);
		if (res != 0) {
			printf("FIFO %s was not created\n",fifo_name);
			exit(EXIT_FAILURE);
		}
	}
}

int open_fifo(const char *fifo_name,int flag,int *fd) {
	*fd = open(fifo_name,flag);
	if (*fd == -1) {
		printf("\nCould not open %s\n in flag %d\n",fifo_name,flag);
		return 0;
	}
	return 1;
}

int main() {
	int res,i,fifo_fd,fd1;
	char buffer[100];
	fd_set my_read;

	signal(SIGKILL,handler);
	signal(SIGINT,handler);
	signal(SIGTERM,handler);

	//init server fifo
	init_server_fifo(REG_FIFO_NAME);
	init_server_fifo(LOGIN_FIFO_NAME);
	init_server_fifo(CHAT_FIFO_NAME);

	//open FIFO for reading
	open_fifo(REG_FIFO_NAME,O_RDONLY,&reg_fifo_fd);
	open_fifo(LOGIN_FIFO_NAME,O_RDONLY,&login_fifo_fd);
	open_fifo(CHAT_FIFO_NAME,O_RDONLY ,&chat_fifo_fd);
    printf("%d %d %d\n",reg_fifo_fd,login_fifo_fd,chat_fifo_fd);	
    FD_ZERO(&my_read);
    FD_SET(reg_fifo_fd,&my_read);
    FD_SET(login_fifo_fd,&my_read);
    FD_SET(chat_fifo_fd,&my_read);

	printf("\nServer is rarin to go\n");
    while(select(chat_fifo_fd + 1,&my_read,NULL,NULL,NULL) == 1) {
         
        if (FD_ISSET(login_fifo_fd, &my_read)) login_client();
        if (FD_ISSET(reg_fifo_fd,&my_read)) register_client();
        if (FD_ISSET(chat_fifo_fd,&my_read)) chat_client();

        FD_SET(reg_fifo_fd,&my_read);
        FD_SET(login_fifo_fd,&my_read);
        FD_SET(chat_fifo_fd,&my_read);
    }
	exit(0);
} 

void register_client() {
    printf("Register Client\n");
    int res;
    int fd;
    char fifoname[BUFF_SZ];

    CHATMSG msg = {"","Welcome!!","Server"};
    while(1) {
        res = read(reg_fifo_fd,&users[user_end],sizeof(USER) );
       if (res != 0) {
            strcpy(fifoname,CLIENT_PREFIX);
            strcat(fifoname,users[user_end].username);
         
            if(open_fifo(fifoname,O_WRONLY | O_NONBLOCK,&fd) == 0) {
                printf("Register Failure\n");
                return ;
            }
            strcpy(msg.to, users[user_end].username);
            write(fd,&msg,sizeof(CHATMSG) );
            user_end++;
            return ;
        }
    }
}
void login_client() {
    printf("Login Client\n");
    int res,fd,user_id;
    USER user;
    char mypipename[100];
    CHATMSG msg = {"","Login Successfully!!","Server"};
    while(1) {
        res = read(login_fifo_fd,&user,sizeof(USER) );
        if (res > 0) break; 
    }
    printf("Read over\n");    
    sprintf(mypipename,"%s%s",CLIENT_PREFIX,user.username);
    if (open_fifo(mypipename,O_WRONLY | O_NONBLOCK,&fd) == 0) {
        printf("Login Failure\n");
        return ;
    }
    
    //check passwd
    user_id = find_user(&user);
//    printf("User:%s\nPasswd:%s\n",user.username,user.passwd);
//    printf("UserId%d\nUsername:%s\nPasswd:%s\n",user_id,user.username,user.passwd);
//    printf("Check Passwd:%d\n",check_passwd(&user,user_id));
    if (user_id < 0) {
        printf("Can not find %s\n",user.username);
        sprintf(msg.message,"Can not find %s\nMaybe Server is wrong!!\n",user.username);
    } else if (check_passwd(&user,user_id) == 0) {
        sprintf(msg.message,"Password is wrong!");
    } else {
        login_users[user_id] = 1;
    }

    strcpy(msg.to,user.username);
    write(fd,&msg,sizeof(CHATMSG) );
    close(fd);
    return ;
}

int find_user(USER *user_ptr) {
    int i = 0;
    while(i < user_end) {
        if (strcmp(user_ptr->username,users[i].username) == 0) return i;
        i++;
    }
    return -1;
}

int check_passwd(USER *user_ptr,int user_id) {
    return strcmp(user_ptr->passwd,users[user_id].passwd) == 0;
}

void chat_client() {
    printf("Chat client\n");
    int res,fd,isFailure = 0;
    CHATMSG msg;
    char fifoname[100];
    while(1) {
        res = read(chat_fifo_fd,&msg,sizeof(CHATMSG) );
        if (res > 0) break;
    }
    sprintf(fifoname,"%s%s",CLIENT_PREFIX,msg.to);
    if (open_fifo(fifoname,O_WRONLY | O_NONBLOCK, &fd) == 0) {
        printf("Chat Failure\n");
        isFailure = 1;
    }
    if (isFailure == 0) {
        write(fd,&msg,sizeof(CHATMSG));
        close(fd);
    }

    sprintf(fifoname,"%s%s",CLIENT_PREFIX,msg.from);
    if (open_fifo(fifoname,O_WRONLY | O_NONBLOCK, &fd) == 0) {
        printf("Chat Failure\n");
        return ;
    }

    if (isFailure) sprintf(msg.message,"Send Message Failure\n");
    else sprintf(msg.message,"Send Message Successfully\n");
    strcpy(msg.to,msg.from);
    strcpy(msg.from,"Server");
    write(fd,&msg,sizeof(CHATMSG) );
    close(fd);
}


