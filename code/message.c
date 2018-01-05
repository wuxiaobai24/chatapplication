#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include "message.h"
/* messenger version */

/***************************************************************************/
/* some local function declaration */

char *get_reply_message(int type);

/* fifo version */
int reciver_init_fifo(messenger_t *messenger, char *path);
int sender_init_fifo(messenger_t *messenger, char *path);
int messenger_destory_fifo(messenger_t *messenger);
int messenger_send_fifo(messenger_t *messenger, void *message,size_t message_size);
int messenger_recive_fifo(messenger_t *messenger,void *messagebuf,size_t message_size);

/* msg queue version */
int reciver_init_msgqueue(messenger_t *messenger, char *parh);
int sender_init_msgqueue(messenger_t *messenger, char *path);
int messenger_destory_msgqueue(messenger_t *messenger);
int messenger_send_msgqueue(messenger_t *messenger, void *message, size_t message_size);
int messenger_recive_msgqueue(messenger_t *messenger, void *message, size_t message_size);

/**********************************************************************/
/* init the messenger */
int messenger_init(messenger_t *messenger, char *path, int type) {
    
    //check parameter
    if (messenger == NULL || path == NULL) return NULL_POINTER;

    messenger->type = type;
#ifdef FIFO_VERSION
    if (type == Reciver) return reciver_init_fifo(messenger,path);
    else if (type == Sender) return sender_init_fifo(messenger,path);
    else return ERROR_TYPE;
#endif

#ifdef MSGQUEUE_VERSION
    if (type == Reciver) return reciver_init_msgqueue(messenger,path);
    else if (type == Sender) return sender_init_msgqueue(messenger,path);
    else return ERROR_TYPE;
#endif
}

/**********************************************************************/
/* destory_messenger */
int messenger_destory(messenger_t *messenger) {
    /* check parameter */
    if (messenger == NULL) return NULL_POINTER;
    
    int res;
#ifdef FIFO_VERSION
    res = messenger_destory_fifo(messenger);
#endif

#ifdef MSGQUEUE_VERSION
    res = messenger_destory_msgqueue(messenger);
#endif

    if (res != 0) return res;

    messenger->type = -1;
    return 0;
}

/***********************************************************************/
/* send message */
int messenger_send(messenger_t *messenger,void *message,size_t message_size) {
    /* check parameter */
    if (messenger == NULL || message == NULL) return NULL_POINTER;
    if (messenger->type != Sender) return ERROR_TYPE;
#ifdef FIFO_VERSION
    return messenger_send_fifo(messenger,message,message_size);
#endif

#ifdef MSGQUEUE_VERSION
    return messenger_send_msgqueue(messenger,message,message_size);
#endif
}

/**********************************************************************/
/* reciver message */
int messenger_recive(messenger_t *messenger, void *messagebuf, size_t message_size) {
    /* check parameter */
    if (messenger == NULL || messagebuf == NULL) return NULL_POINTER;
    if (messenger->type != Reciver) return ERROR_TYPE;

#ifdef FIFO_VERSION
    return messenger_recive_fifo(messenger,messagebuf,message_size);
#endif

#ifdef MSGQUEUE_VERSION
    return messenger_recive_msgqueue(messenger,messagebuf,message_size);
#endif

}




/**********************************************************************/
/* init reciver -- fifo version */
int reciver_init_fifo(messenger_t *messenger, char *path) {
    /* create fifo */
    int res;
    
    /* we don't create fifo, so we can set the sigal handler between create
     * fifo and open fifo, for example:
     *
     * mkfifo(path,0777);               //create fifo
     * signal(SKGINT,cleaner);          // set signal handler
     * messenger_init(messenger,path);  // open fifo 
     */

//    res = mkfifo(path,0777);
//    if (res != 0) return FATAL_ERROR;

    /* open fifo */
  //  printf("Open read fifo:%s\n",path);
    res = open(path, O_RDONLY);
    if (res == -1) return FATAL_ERROR;

    messenger->id = res;
    return 0;
}

/**********************************************************************/
/* init sender -- fifo version */
int sender_init_fifo(messenger_t *messenger, char *path) {
    /* open fifo */
    int res;
    
    printf("Open write fifo:%s\n",path);
    res = open(path,O_WRONLY);
    if (res == -1) return FATAL_ERROR;

    messenger->id = res;
    return 0;
}

/********************************************************************/
/* destory messenger -- fifo version */
int messenger_destory_fifo(messenger_t *messenger) {
    int res;
    
    res = close(messenger->id);
    if (res != 0) return FATAL_ERROR;

    messenger->id = -1;
    return 0;
}

/*******************************************************************/
/* send message -- fifo version */
int messenger_send_fifo(messenger_t *messenger,void *message,size_t message_size) {
  //  printf("fifo send\n");
    return write(messenger->id,message,message_size);
}

/********************************************************************/
/* recive message -- fifo version */
int messenger_recive_fifo(messenger_t *messenger, void *messagebuf,size_t message_size) {
    //printf("fifo recive\n");
    return read(messenger->id,messagebuf,message_size);
}



/*******************************************************************/
/* init the server reply */
int init_reply(chat_message_t *reply_buf, int type) {
    if (reply_buf == NULL) return NULL_POINTER;
    if (type > ParseError) return ERROR_TYPE;
    
    strcpy(reply_buf->sender,SERVER_NAME);
    strcpy(reply_buf->message,get_reply_message(type));
    return 0;

}

/*******************************************************************/
/* get the reply message by reply_type */
char *get_reply_message(int reply_type) {
    switch(reply_type) {
        case SuccessReply:
            return "Success";
            //break;  //in fact, we don't nead break
        case WrongUserName:
            return "WrongUserName";
        case ServerError:
            return "ServerError";
        case NoUser:
            return "NoUser";
        case WrongPasswd:
            return "WrongPasswd";
        case WrongSender:
            return "WrongSender";
        case UserIsNotLoggedIn:
            return "UserIsNotLoggedIn";
        case WrongReciver:
            return "WrongReciver";
        case UserIsTooMuch:
            return "UserIsTooMuch";
        default: //ParseError
            return "ParseError";
    }
}

/*******************************************************************/
/* parse the server reply */
int parse_server_reply(chat_message_t *reply_buf) {

    int i;
    for(i = 0;i < ParseError;i++) {
        if (strcmp(reply_buf->message,get_reply_message(i)) == 0)
            return i;
    }
    return ParseError;
}

/********************************************************************/
/* username to path */

int username2path(char *username,char *path,int type) {
    if (username == NULL) return NULL_POINTER;
        
//    printf("username2path:username = %s\n",username);

    switch (type) {
        case Client:
            sprintf(path,"%s%sc",CLIENT_FILE_PREFIX,username);
            break;
        case Temp:
            sprintf(path,"%s%st",CLIENT_FILE_PREFIX,username);
            break;
        case Server:
            sprintf(path,"%s%ss",SERVER_FILE_PREFIX,username);
            break;
        default:
            return ERROR_TYPE;
    }
    return 0;
}

void message_show(chat_message_t *msg) {
    printf("Sender:%s\n",msg->sender);
    printf("Reciver:%s\n",msg->reciver);
    printf("Message:%s\n",msg->message);
}



/**********************************************************************/
/* message queue version */

/**********************************************************************/
/* init a reciver -- msg queue version */
int reciver_init_msgqueue(messenger_t *messenger, char *path) {
    int res;
    key_t key = ftok(path,'a');
    res = msgget(key,IPC_CREAT | 0666);
    if (res == -1) return FATAL_ERROR;

    messenger->id = res;
    return 0;
}

/*********************************************************************/
/* init a sender -- msg queue version */

int sender_init_msgqueue(messenger_t *messenger, char *path) {
    //just call reciver init
    return reciver_init_msgqueue(messenger,path);
}

/********************************************************************/
/* destory messenger -- msg queue version */
int messenger_destory_msgqueue(messenger_t *messenger) {
    /* maybe we shouldn't delete the msgqueue */
    
//    msgctl(messenger->id, IPC_RMID, 0);

    return 0;
}

#define HELPER_SIZE 300

typedef struct {
    long type;
    char data[HELPER_SIZE];
} msgqueue_helper;


/**********************************************************************/
/* send a message -- msg queue version */
int messenger_send_msgqueue(messenger_t *messenger, void *message, size_t msg_size) {

    /* msgsnd success will return 0, not message size, so we should do
     * something , make it like fifo */
//    printf("send msg, id is %d\n",messenger->id);
    ((chat_message_t*)message)->msgqueue_type = 1;
    int res = msgsnd(messenger->id, message, HELPER_SIZE, 0);
    if (res == -1) return -1;
    return msg_size;
}

/**********************************************************************/
/* recive s message -- msg queue version */
int messenger_recive_msgqueue(messenger_t *messenger, void *message, size_t msg_size) {
//    printf("reciver msg, id is %d,msg_size is %lu\n",messenger->id,msg_size);
//    printf("message_size is %lu\n",sizeof(chat_message_t));
    int res = msgrcv(messenger->id, message, HELPER_SIZE, 0, 0);
    if (res == HELPER_SIZE) return msg_size;
    return res;
}
