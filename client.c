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
int logged = 0;
char user_name[100];

void handler(int sig) {
	unlink(mypipename);
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
void login_client(int login_fifo_fd,int my_fifo,char *myfifoname);
void receive_information();
void chat_client(int chat_fifo_fd,int myfifo,char *myfifoname);
int main(int argc,char *argv[] ) {
	int res;
	int fifo_fd,my_fifo;
	int fd;
	CLIENTINFO info;
	char buffer[BUFF_SZ];
	int reg_fifo_fd,login_fifo_fd,chat_fifo_fd;
	int action;
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
	
	while(1) {
		printf("1.Register\n");
		printf("2.Login\n");
		printf("3.Chat\n");
		if (logged){
			printf("User %s logged\n",user_name);
			receive_information(my_fifo);
		}
		printf(">");
		scanf("%d",&action);
		if (action==1) register_client(reg_fifo_fd,my_fifo,mypipename);
		else if (action == 2) login_client(login_fifo_fd,my_fifo,mypipename);
		else if (action == 3) chat_client(chat_fifo_fd,my_fifo,mypipename);
	}
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
	int res,sig;

	//construction register msg
	printf("Please enter you name:\n");
	scanf("%s",reg_msg.name);
	printf("Please enter you passwd:\n");
	scanf("%s",reg_msg.passwd);
	strcpy(reg_msg.myfifo,myfifoname);
	
	write(reg_fifo_fd,&reg_msg,sizeof(REGMSG) );
	while(1) {
		res = read(my_fifo,&sig,sizeof(int));
		if (res > 0) {
			if (sig == 10 )
				printf("Register failure!!\n");
			else if (sig == 11)
				printf("Register successfully!!\n");
			else
				printf("Server error:%d\n",sig);
			return;
		}
	}
}

void login_client(int login_fifo_fd,int my_fifo,char *myfifoname) {
	LOGINMSG login_msg;
	char buffer[BUFF_SZ];
	int sig;
	int res;

	printf("Please enter your name:\n");
	scanf("%s",login_msg.name);
	printf("Please enter your passwd:\n");
	scanf("%s",login_msg.passwd);
	strcpy(login_msg.myfifo,myfifoname);
	write(login_fifo_fd,&login_msg,sizeof(LOGINMSG) );


	printf("wait form login server\n");
	while(1) {
		res = read(my_fifo,&sig,sizeof(int));
		if (res != 0) {
			if (sig == 20)
				printf("Login faliure:\tCan not find the user %s\n",login_msg.name);
			else if (sig == 21)
				printf("Login faliure:\tPassword error!!\n");
			else if (sig == 22)
				printf("Login faliure:\tUser %s is logged\n",login_msg.name);
			else if (sig == 23) {
				printf("Login successfully\n");
				strcpy(user_name,login_msg.name);
				logged = 1;
			}
			else
				printf("Server error:%d\n",sig);
			break;
		}
	}
}

void chat_client(int chat_fifo_fd,int my_fifo,char *myfifoname) {
	int sig;
	int res;
	CHATMSG msg;
	strcpy(msg.fromfifo,myfifoname);
	printf("From:%s\nTo:",user_name);
	scanf("%s",msg.to);
	printf("\nMessage:\n");
	scanf("%s",msg.message);
	write(chat_fifo_fd,&msg,sizeof(CHATMSG) );
	while(1) {
		res = read(my_fifo,&sig,sizeof(int) );
		if (res != 0) break; 
	}
	if (sig & 1) printf("Error: Server can not find you!!\nMaybe you are not logged!\n");
	if (sig & 2) printf("Error: Can find %s\n",msg.to);
	if (sig & 4) printf("Error: User %s is not logged!\n",msg.to);
	if (sig == 0) printf("Send message successfully\n");
}
void receive_information(int myfifo) {
	int res;
	int sig = 0;
	int i = 0;
	CHATMSG msg;
	printf("Receive_information\n");
	for(i = 0;i < 1000;i++) {
		res = read(myfifo,&sig,sizeof(int));
		if (res > 0 && sig == 9) break;
	}
	printf("sig:%d",sig);
	if (sig != 9) return ;
	for(i = 0;i < 1000;i++) {
		res = read(myfifo,&msg,sizeof(CHATMSG) );
		if (res > 0) {
			printf("From:%s\nTo:%s\nMessage:\n%s\n",msg.fromfifo,msg.to,msg.message);
			break;
		}
	}
}
