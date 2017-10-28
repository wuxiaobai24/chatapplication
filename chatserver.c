#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include "clientinfo.h"
#include <signal.h>
#include <stdio.h>

#define CLIENT_COUNT 20
void handler(int sig) {
	unlink(REG_FIFO_NAME);
	unlink(LOGIN_FIFO_NAME);
	unlink(CHAT_FIFO_NAME);
	exit(1);
}

void register_client(REGMSG* reg_msg_p);
void login_client(LOGINMSGPTR login_msg_p);
void chat_client(CHATMSG* chat_msg_p);

REGMSG clients[CLIENT_COUNT];
int end = 0;
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

int open_fifo(const char *fifo_name,int flag) {
	int fd = open(fifo_name,flag);
	if (fd == -1) {
		printf("\nCould not open %s\n in flag %d\n",fifo_name,flag);
		exit(EXIT_FAILURE);
	}
	return fd;
}

int main() {
	int res,i,fifo_fd,fd1;
	int reg_fifo_fd;
	int login_fifo_fd;
	int chat_fifo_fd;
	//CLIENTINFO info;
	REGMSG   reg_msg;
	LOGINMSG login_msg;
	CHATMSG chat_msg;
	char buffer[100];
	
	signal(SIGKILL,handler);
	signal(SIGINT,handler);
	signal(SIGTERM,handler);

	//init server fifo
	init_server_fifo(REG_FIFO_NAME);
	init_server_fifo(LOGIN_FIFO_NAME);
	init_server_fifo(CHAT_FIFO_NAME);

	//open FIFO for reading
	reg_fifo_fd = open_fifo(REG_FIFO_NAME,O_RDONLY);
	login_fifo_fd = open_fifo(LOGIN_FIFO_NAME,O_RDONLY);
	chat_fifo_fd = open_fifo(CHAT_FIFO_NAME,O_RDONLY);
	
	printf("\nServer is rarin to go\n");

	while(1) {
		//register
		res = read(reg_fifo_fd,&reg_msg,sizeof(REGMSG) );
		if (res != 0) {
			register_client(&reg_msg);
		}
		//login
	/*	res = read(login_fifo_fd,&login_msg,sizeof(LOGINMSG) );
		if (res != 0) {
			login_client(&login_msg);
		}
		//chat
		res = read(chat_fifo_fd,&chat_msg,sizeof(CHATMSG));
		if (res != 0) {
			chat_client(&chat_msg);
		}
	*/
	}	
	exit(0);
} 

void register_client(REGMSG* reg_msg_p) {
	printf("Register!!!\n");
	int i = 0,fd;
	char buffer[100];
	while(i < end && strcmp(reg_msg_p->name,clients[i].name) != 0 )
		i++;

	if (i == end){
		clients[i] = *reg_msg_p;
		sprintf(buffer,"Registerd successfully!!\n");
	} else {
		sprintf(buffer,"Registerd failed!!\n");			
	}
	
	fd = open(reg_msg_p->myfifo,O_WRONLY | O_NONBLOCK);
	if (fd == -1) {
		printf("\nColud not open %s\n",reg_msg_p->myfifo);
		return ;
	}
	write(fd,buffer,sizeof(buffer) );
	close(fd);
}
void login_client(LOGINMSGPTR login_msg_p) {
	printf("Login!!\n");
}

void chat_client(CHATMSG *chat_msg_p) {
	printf("Chat!!\n");
}
