#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include "clientinfo.h"
#include <signal.h>
#include <stdio.h>

//#define FIFO_NAME "/home/wukunhan2015170297/chatapplication/data/server_fifo/chat_server_fifo"
#define FIFO_NAME "/tmp/server_fifo"

void handler(int sig) {
	unlink(FIFO_NAME);
	exit(1);
}

int calc(CLIENTINFOPTR info) {
	switch(info->op) {
		case '+': return info->leftarg + info->rightarg;
		case '-': return info->leftarg - info->rightarg;
		case '*': return info->leftarg * info->rightarg;
		case '/': return info->leftarg / info->rightarg;
	}
	return 0;
}

int main() {
	int res,i,fifo_fd,fd1;
	CLIENTINFO info;
	char buffer[100];
	
	signal(SIGKILL,handler);
	signal(SIGINT,handler);
	signal(SIGTERM,handler);
	
	if (access(FIFO_NAME,F_OK) == -1 ) {
		res = mkfifo(FIFO_NAME,0777);
		if (res != 0) {
			printf("FIFO %s was not created\n",FIFO_NAME);
			exit(EXIT_FAILURE);
		}
	}

	//open FIFO for reading
	fifo_fd = open(FIFO_NAME, O_RDONLY);
	if (fifo_fd == -1 ) {
		printf("\nCould not open %s for read only access\n",FIFO_NAME);
		exit(EXIT_FAILURE);
	}
	
	printf("\nServer is rarin to go\n");

	while(1) {
		res = read(fifo_fd,&info,sizeof(CLIENTINFO) );
		if (res != 0) {
			printf("Client arrived!!\n");
			sprintf(buffer,"The result is %d",calc(&info) );
			fd1 = open(info.myfifo, O_WRONLY | O_NONBLOCK);
			if (fd1 == -1) {
				printf("\nCould not open %s\n",info.myfifo);
			}
			write(fd1,buffer,strlen(buffer) +1);
			close(fd1);
		}
	}	
	exit(0);
} 
