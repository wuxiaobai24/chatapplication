#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "clientinfo.h"
#include <signal.h>
#include <stdio.h>

#define CLIENT_COUNT 20
#define BUFF_SZ 100
int reg_fifo_fd, login_fifo_fd, chat_fifo_fd;
USER users[CLIENT_COUNT];
int user_end = 0;
int login_users[CLIENT_COUNT];


void handler(int sig) {
	unlink(REG_FIFO_NAME);
	unlink(LOGIN_FIFO_NAME);
	unlink(CHAT_FIFO_NAME);
	exit(1);
}

void register_client();
void login_client();
void chat_client();

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
	
    FD_SET(reg_fifo_fd,&my_read);
    FD_SET(login_fifo_fd,&my_read);
    FD_SET(chat_fifo_fd,&my_read);

	printf("\nServer is rarin to go\n");
    while(select(chat_fifo_fd + 1,&my_read,NULL,NULL,NULL) == 1) {
        
        if (FD_ISSET(reg_fifo_fd,&my_read)) register_client();
        if (FD_ISSET(login_fifo_fd, &my_read)) login_client();
        if (FD_ISSET(chat_fifo_fd,&my_read)) chat_client();
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
        if (res != 0) break; 
    }
    
    sprintf(mypipename,"%s%s",CLIENT_PREFIX,user.username);
    if (open_fifo(mypipename,O_WRONLY | O_NONBLOCK,&fd) == 0) {
        printf("Login Failure\n");
        return ;
    }
    
    //check passwd
    user_id = find_user(&user);
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
    while(i < user_end)
        if (strcmp(user_ptr->username,users[i].username) == 0) return i;
    return -1;
}

int check_passwd(USER *user_ptr,int user_id) {
    return strcmp(user_ptr->passwd,users[user_id].passwd);
}

void chat_client() {

}


