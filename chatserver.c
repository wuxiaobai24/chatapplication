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
int logged_clients[CLIENT_COUNT] = {0};
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
	reg_fifo_fd = open_fifo(REG_FIFO_NAME,O_RDONLY | O_NONBLOCK);
	login_fifo_fd = open_fifo(LOGIN_FIFO_NAME,O_RDONLY | O_NONBLOCK);
	chat_fifo_fd = open_fifo(CHAT_FIFO_NAME,O_RDONLY | O_NONBLOCK);
	
	printf("\nServer is rarin to go\n");

	while(1) {
		//register
		res = read(reg_fifo_fd,&reg_msg,sizeof(REGMSG) );
		if (res > 0) {
			register_client(&reg_msg);
		}
		//login
		res = read(login_fifo_fd,&login_msg,sizeof(LOGINMSG) );
		//printf("login res %d\n",res);
		if (res > 0) {
			printf("res = %d\n",res);
			login_client(&login_msg);
		}
		//chat
		res = read(chat_fifo_fd,&chat_msg,sizeof(CHATMSG));
		if (res > 0) {
			chat_client(&chat_msg);
		}
	
	}	
	exit(0);
} 

void register_client(REGMSG* reg_msg_p) {
	printf("Register!!!\n");
	int i = 0,fd;
	int sig = 10; //register failure
	//search
	while(i < end && strcmp(reg_msg_p->name,clients[i].name) != 0 )
		i++;

	if (i == end){
		clients[end++] = *reg_msg_p;
		sig = 11; // register successfully
	}

	//open client fifo	
	fd = open(reg_msg_p->myfifo,O_WRONLY | O_NONBLOCK);
	if (fd == -1) {
		printf("\nColud not open %s\n",reg_msg_p->myfifo);
		return ;
	}
	
	//send a signal to client
	write(fd,&sig,sizeof(int) );
	close(fd);
	printf("end=%d\n,sig=%d",end,sig);
}

void login_client(LOGINMSGPTR login_msg_p) {
	printf("Login!!\n");
	int i = 0,fd;
	int sig = 20; // login failure

	//search
	while(i < end && strcmp(login_msg_p->name,clients[i].name) != 0)
		i++;
	
	if (i == end)
		sig = 20; // can not find the user 
	else if (strcmp(login_msg_p->passwd,clients[i].passwd) != 0)
		sig = 21; //passwd wrong
	else if (logged_clients[i]) 
		sig = 22; //client is logged
	else {
		sig = 23;//login successfully
		logged_clients[i] = 1;
		//update client fifo
		if (strcmp(login_msg_p->myfifo,clients[i].myfifo) != 0)
			strcpy(clients[i].myfifo,login_msg_p->myfifo);
	}
	
	//open client fifo
	fd = open(login_msg_p->myfifo,O_WRONLY | O_NONBLOCK);
	if (fd == -1) {
		printf("\nCould not open %s\n",clients[i].myfifo);
		return ;
	}
	
	write(fd,&sig,sizeof(int) );
	close(fd);
}

void chat_client(CHATMSG *chat_msg_p) {
	printf("Chat:!!\n");
	int from = 0,to = 0;
	int sig = 0,fd;
	while(from  < end && strcmp(chat_msg_p->fromfifo,clients[from].myfifo) != 0)
		from++;
	while(to < end && strcmp(chat_msg_p->to,clients[to].name) != 0)
		to++;
	
	if (from == end) sig |= 1;
	if (to == end) sig != 2;
	if (logged_clients[to] == 0) sig != 4;

	fd = open(chat_msg_p->fromfifo,O_WRONLY | O_NONBLOCK);
	printf("fd=%d\n",fd);
	if (fd == -1) {
		printf("\nCould not open %s \n",chat_msg_p->fromfifo);
		return ;
	}
	write(fd,&sig,fd);
	close(fd);

	if (sig != 0) return ;
	printf("Open fifo:%s\n",clients[to].myfifo);
	fd = open(clients[to].myfifo,O_WRONLY | O_NONBLOCK);
	printf("fd=%d\n",fd);
	if (fd == -1) {
		printf("\nCould not open %s\n",clients[to].myfifo);
		return ;
	}
	strcpy(chat_msg_p->fromfifo,clients[from].name);
	sig = 9;
	write(fd,&sig,fd);
	write(fd,chat_msg_p,sizeof(CHATMSG) );
	return ;
}
