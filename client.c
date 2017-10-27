#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include "clientinfo.h"
#include <signal.h>

#define FIFO_NAME "/home/wukunhan2015170297/chatapplication/data/server_fifo/chat_server_fifo"
//#define FIFO_NAME "/tmp/server_fifo"
#define BUFF_SZ 100

char mypipename[BUFF_SZ];

void handler(int sig) {
	unlink(mypipename);
	exit(1);
}

int main(int argc,char *argv[] ) {
	int res;
	int fifo_fd,my_fifo;
	int fd;
	CLIENTINFO info;
	char buffer[BUFF_SZ];
	
	signal(SIGKILL,handler);
	signal(SIGINT,handler);
	signal(SIGTERM,handler);
	
	if (argc != 4) {
		printf("Usage: %s opl operation op2\n",argv[0] );
		exit(1);
	}
	
	if (access(FIFO_NAME,F_OK) == -1) {
		printf("Could not open FIFO %s \n",FIFO_NAME);
		exit(EXIT_FAILURE);
	}
	
	//open server fifo for write
	fifo_fd = open(FIFO_NAME, O_WRONLY);
	if (fifo_fd == -1) {
		printf("Could not open %s for write access\n",FIFO_NAME);
		exit(EXIT_FAILURE);
	}
	
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
//	printf("#")	
	//construct client info
	strcpy(info.myfifo,mypipename);
	info.leftarg = atoi(argv[1] );
	info.op = argv[2][0];
	info.rightarg = atoi(argv[3]);
	//printf("info.myfifo:%s\n",info.myfifo);
	write(fifo_fd, &info,sizeof(CLIENTINFO) );
	close(fifo_fd);

	memset(buffer,'\0',BUFF_SZ);
	printf("#\n");
	printf("info.myfifo:%s\n",info.myfifo);
	printf("info.leftarg:%d\n",info.leftarg);
	printf("info.op:%c\n",info.op);
	printf("info.rightart:%d\n",info.rightarg);
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
