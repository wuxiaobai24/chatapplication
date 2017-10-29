#ifndef _CLIENTINFO_H
#define _CLIENTINFO_H

#define REG_FIFO_NAME "/home/wukunhan2015170297/chatapplication/data/server_fifo/server_regiser_fifo"
#define LOGIN_FIFO_NAME "/home/wukunhan2015170297/chatapplication/data/server_fifo/server_login_fifo"
#define CHAT_FIFO_NAME "/home/wukunhan2015170297/chatapplication/data/server_fifo/server_chat_fifo"

typedef struct{
	char myfifo[100];
	char name[100];
	int id;
} CLIENTINFO,* CLIENTINFOPTR;

typedef struct{
	char name[100];
	char passwd[100];
	char myfifo[100];
} REGMSG, *REGMESPTR;

typedef struct{
	char name[100];
	char passwd[100];
	char myfifo[100];
}LOGINMSG, *LOGINMSGPTR;

typedef struct {
	char to[100];
	char message[100];
	char fromfifo[100];
} CHATMSG, *CHATMSGPTR;

#endif
