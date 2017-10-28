#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include "clientinfo.h"
#include <signal.h>

//#define FIFO_NAME "/home/wukunhan2015170297/chatapplication/data/server_fifo/chat_server_fifo"

#define BUFF_SZ 100

char mypipename[BUFF_SZ];

void handler(int sig) {
//	unlink(mypipename);
	exit(1);
}
int open_server_fifo(const char *fifo_name) {
	int fifo_fd;
	if (access(fifo_name,F_OK) == -1) {
		printf("Could not open FIFO %s \n",fifo_name);
		exit(EXIT_FAILURE);
	}
	
	//open server fifo for write
	fifo_fd = open(fifo_name, O_WRONLY);
	if (fifo_fd == -1) {
		printf("Could not open %s for write access\n",fifo_name);
		exit(EXIT_FAILURE);
	}
	return fifo_fd;
}

void register_client(int reg_fifo_fd,int my_fifo,char *myfifoname);

int main(int argc,char *argv[] ) {
	int res;
	int fifo_fd,my_fifo;
	int fd;
	CLIENTINFO info;
	char buffer[BUFF_SZ];
	int reg_fifo_fd,login_fifo_fd,chat_fifo_fd;
	
	signal(SIGKILL,handler);
	signal(SIGINT,handler);
	signal(SIGTERM,handler);
	
	reg_fifo_fd = open_server_fifo(REG_FIFO_NAME);
	login_fifo_fd = open_server_fifo(LOGIN_FIFO_NAME);
	chat_fifo_fd = open_server_fifo(CHAT_FIFO_NAME);
	
	//create my own FIFO
	sprintf(mypipename,
	"/home/wukunhan2015170297/chatapplication/data/client_fifo/chat_client%d_fifo",getpid() );

//	sprintf(mypipename,"/tmp/client%d_fifo",getpid() );
	res = mkfifo(mypipename,0777);
	if (res != 0 ) {
		printf("FIFO %s was not created\n",mypipename);	//buffer);?
		exit(EXIT_FAILURE);
	}
	
	//ope my own FIFO for reading
	my_fifo = open(mypipename, O_RDONLY | O_NONBLOCK);//O_NOBLOCK?
	if (my_fifo == -1) {
		printf("Could not open %s for read only access\n",mypipename);	//FIFONAME?
		exit(EXIT_FAILURE);
	}
	register_client(reg_fifo_fd,my_fifo,mypipename);
	return 0;
	//construct client info
	strcpy(info.myfifo,mypipename);
	printf("Please enter your name:\n");
	scanf("%s",info.name);
//	info.pid = getpid();

	printf("info.myfifo:%s\n",info.myfifo);
	printf("info.name:%s\n",info.name);
//	printf("info.pid:%d\n",info.pid);

	write(fifo_fd, &info,sizeof(CLIENTINFO) );
	close(fifo_fd);

	memset(buffer,'\0',BUFF_SZ);
	while(1) {
		res = read(my_fifo,buffer,BUFF_SZ);
		if (res > 0 ) {
			printf("Received from server: %s\n",buffer);
			break;
		}
	}
	printf("Client %d is terminating\n",getpid() );
	close(my_fifo);
	(void)unlink(mypipename);
	exit(0);
}

void register_client(int reg_fifo_fd,int my_fifo,char *myfifoname) {
	char buffer[100];
	REGMSG reg_msg;
	int res;

	printf("Please enter you name:\n");
	scanf("%s",reg_msg.name);
	printf("Please enter you passwd:\n");
	scanf("%s",reg_msg.passwd);
	//reg_msg.myfifo = myfifoname;
	strcpy(reg_msg.myfifo,myfifoname);
	write(reg_fifo_fd,&reg_msg,sizeof(REGMSG) );
	while(1) {
		res = read(my_fifo,buffer,BUFF_SZ);
		if (res > 0) {
			printf("Received from server:%s\n",buffer);
			break;
		}
	}
}

