#ifndef _MESSAGE_H
#define _MESSAGE_H

#define MESSAGE_BUFFER_SZ 100

/* RETUNR VALUE */
#define FATAL_ERROR -1
#define NULL_POINTER -2
#define ERROR_TYPE -3
#define SUCCESS 0


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

#endif
