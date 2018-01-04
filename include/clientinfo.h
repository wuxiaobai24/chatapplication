#ifndef _CLIENTINFO_H
#define _CLIENTINFO_H

#define REG_FIFO_NAME "/home/wukunhan2015170297/chatapplication/data/server_fifo/server_regiser_fifo"
#define LOGIN_FIFO_NAME "/home/wukunhan2015170297/chatapplication/data/server_fifo/server_login_fifo"
#define CHAT_FIFO_NAME "/home/wukunhan2015170297/chatapplication/data/server_fifo/server_chat_fifo"
#define CLIENT_PREFIX "/home/wukunhan2015170297/chatapplication/data/client_fifo/clinet_fifo_"

typedef struct {
    char username[20];
    char passwd[30];
} USER, *USERPTR;

typedef struct {
	char to[20];
	char message[100];
	char from[20];
} CHATMSG, *CHATMSGPTR;

#endif
