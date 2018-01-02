#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "message.h"

/* messenger version */
#define FIFO

/***************************************************************************/
/* some local function declaration */

char *get_reply_message(int type);

/* fifo version */
int reciver_init_fifo(messenger_t *messenger, char *path);
int sender_init_fifo(messenger_t *messenger, char *path);
int messenger_destory_fifo(messenger_t *messenger);
int messenger_send_fifo(messenger_t *messenger, void *message,size_t message_size);
int messenger_recive_fifo(messenger_t *messenger,void *messagebuf,size_t message_size);
/**********************************************************************/
/* init the messenger */
int messenger_init(messenger_t *messenger, char *path, int type) {
    
    //check parameter
    if (messenger == NULL || path == NULL) return NULL_POINTER;

    messenger->type = type;
    if (type == Reciver) return reciver_init_fifo(messenger,path);
    else if (type == Sender) return sender_init_fifo(messenger,path);
    else return ERROR_TYPE;
}

/**********************************************************************/
/* destory_messenger */
int messenger_destory(messenger_t *messenger) {
    /* check parameter */
    if (messenger == NULL) return NULL_POINTER;
    
    int res;
    res = messenger_destory_fifo(messenger);
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
    printf("Send message\n");
    return messenger_send_fifo(messenger,message,message_size);
}

/**********************************************************************/
/* reciver message */
int messenger_recive(messenger_t *messenger, void *messagebuf, size_t message_size) {
    /* check parameter */
    if (messenger == NULL || messagebuf == NULL) return NULL_POINTER;
    if (messenger->type != Reciver) return ERROR_TYPE;
    printf("Recive message\n");
    return messenger_recive_fifo(messenger,messagebuf,message_size);
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
    printf("Open read fifo:%s\n",path);
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
    return write(messenger->id,message,message_size);
}

/********************************************************************/
/* recive message -- fifo version */
int messenger_recive_fifo(messenger_t *messenger, void *messagebuf,size_t message_size) {
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
        
    printf("username2path:username = %s\n",username);

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
