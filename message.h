#ifndef _MESSAGE_H
#define _MESSAGE_H

#define MESSAGE_BUFFER_SZ 100

/* RETUNR VALUE */
#define FATAL_ERROR -1
#define NULL_POINTER -2
#define ERROR_TYPE -3
#define SUCCESS 0

#define SERVER_NAME "CHATSERVER"
#define CLIENT_FILE_PREFIX "/home/wukunhan2015170297/chatapplication/data/"
#define SERVER_FILE_PREFIX "/home/wukunhan2015170297/chatapplication/data/"

enum messenger_type{
    Reciver,
    Sender
};

typedef struct {
    int id;
    int type; 
}messenger_t;

typedef struct {
    char username[MESSAGE_BUFFER_SZ];
    char passwd[MESSAGE_BUFFER_SZ];
    char temp_reciver[MESSAGE_BUFFER_SZ];
}register_message_t;

/* login and register use same type */
typedef register_message_t login_message_t;

typedef struct {
    char sender[MESSAGE_BUFFER_SZ];
    char reciver[MESSAGE_BUFFER_SZ];
    char message[MESSAGE_BUFFER_SZ];
} chat_message_t;

/* use same interface, so we can easy change the fifo to message queue */

/* init the messenger with path and type */
int messenger_init(messenger_t *messenger,char *path,int type);
/* destory messenger */
int messenger_destory(messenger_t *messenger);
/* seng message by messenger */
int messenger_send(messenger_t *messenger,void *message,size_t message_size);
/* recive message by messenger */
int messenger_recive(messenger_t *messenger,void *messagebuf,size_t message_size);

/***************************************************************************/
/* some function and enum which is help for init reply and parse reply */

enum reply_type{
    SuccessReply = 0, WrongUserName, ServerError,NoUser, WrongPasswd, UserIsLoggedIn, 
    ParseError // the parse error must be the last one, check type will use it
};

int init_reply(chat_message_t *reply_buf, int type);
int parse_server_reply(chat_message_t *reply_buf);


/****************************************************************************/
/* some function for init the user filename */
enum userfile_type {
    Client, Temp, Server
};
/* create the userfile , the path will return by username parameter,
 * so please make sure the username have enought space */
int username2path(char *username,char *path,int type);


/***************************************************************************/
/* show chat message is for debug */
void message_show(chat_message_t *msg);

#endif
