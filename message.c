#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "message.h"

/* messenger version */
#define FIFO

/***************************************************************************/
/* some local function declaration */

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
int destory_messenger(messenger_t *messenger) {
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
    if (messenger->type != Reciver) return ERROR_TYPE;

    return messenger_send_fifo(messenger,message,message_size);
}

/**********************************************************************/
/* reciver message */
int messenger_recive(messenger_t *messenger, void *messagebuf, size_t message_size) {
    /* check parameter */
    if (messenger == NULL || messagebuf == NULL) return NULL_POINTER;
    if (messenger->type != Reciver) return ERROR_TYPE;

    return messenger_recive(messenger,messagebuf,message_size);
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

