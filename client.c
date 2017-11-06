#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "clientinfo.h"
#include <signal.h>

//#define FIFO_NAME "/home/wukunhan2015170297/chatapplication/data/server_fifo/chat_server_fifo"

#define BUFF_SZ 100

char mypipename[BUFF_SZ];
int logged = 0;
char user_name[100];
int reg_fifo_fd,login_fifo_fd,chat_fifo_fd;
int client_fifo_fd;

void print_chat_msg(CHATMSGPTR msg_ptr);
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

void register_client();

int main(int argc,char *argv[] ) {
	int res;
	int fifo_fd,my_fifo;
	int fd;
	char buffer[BUFF_SZ];
	int action;
	signal(SIGKILL,handler);
	signal(SIGINT,handler);
	signal(SIGTERM,handler);
	
	reg_fifo_fd = open_server_fifo(REG_FIFO_NAME);
	login_fifo_fd = open_server_fifo(LOGIN_FIFO_NAME);
	chat_fifo_fd = open_server_fifo(CHAT_FIFO_NAME);
	
	while(1) {
		printf("1.Register\n");
		printf("2.Login\n");
		printf("3.Chat\n");
		printf(">");
		scanf("%d",&action);
		if (action==1) register_client();
		else if (action == 2); 
		else if (action == 3);
	}
	return 0;
}

void register_client() {
    USER user;
    int res;
    CHATMSG msg;

	//construction register msg
    int flag = 0;
    do {
        if (flag) printf("Username %s is exist\n",user.username);
	    printf("Please enter you name:\n");
	    scanf("%s",user.username);
        strcpy(mypipename,CLIENT_PREFIX);
        strcat(mypipename,user.username);
        flag = 1;
    } while( access(mypipename,F_OK) != -1);

	printf("Please enter you passwd:\n");
	scanf("%s",user.passwd);

    //create client_fiifo
    res = mkfifo(mypipename,0777);
    if (res != 0) {
        printf("FIFO %s was not created\n",mypipename);
        return ;
    }
    //open client_fifo
    client_fifo_fd = open(mypipename,O_RDONLY | O_NONBLOCK);
    if (client_fifo_fd == -1) {
        printf("Could not open %s.\n",mypipename);
        return ;
    }
    
    printf("Send to Register Message to Server\n");
	
    write(reg_fifo_fd,&user,sizeof(USER) );
    while(1) {
        res = read(client_fifo_fd,&msg,sizeof(CHATMSG) );
        if (res != 0) {
            print_chat_msg(&msg);
            return ;
        }
    }
}

void print_chat_msg(CHATMSGPTR msg_ptr) {
    printf("\nFrom:%s\nMSG:%s\n",msg_ptr->from,msg_ptr->message);
}
