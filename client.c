#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "user.h"
#include "config.h"
#include "message.h"

/* some macro definition */
#define BUFF_SZ 100
#define CONFIGURATIONFILE_PATH "/home/wukunhan2015170297/.chatapplication"
/****************************************************************************/
/* global val */

/* login user information */
user_t *logged_in_user;
messenger_t *user_reciver;


/* server messenger */
messenger_t *reg_sender;
messenger_t *login_sender;
messenger_t *msg_sender; /* chat messeng */

/* client temp reciver */
char client_temp_name[BUFF_SZ];
messenger_t *temp_reciver;

/* config */
config_t *config;

/****************************************************************************/
enum {
    Login,Register,SendMsg,ReciveMsg,DoNothing
};

/****************************************************************************/
/* function declaration */
int main(int argc,char **argv);         /* the main function */
void init();                            /* init the client */
void fatalError(char *prompt);          /* fatal error */
void listen_loop();                     /*listen loop, wait user input*/
void register_user();              /* register */
void login_user();                 /* login user */
void send_msg();                        /* send msg */
void recive_msg();                      /* recive_msg */
void init_sender();                     /* init the messenger for server */
void init_temp_reciver();               /* init temp reciver for client */
void init_config();                     /* init the config */
int show_prompt_with_user();            /* show prompt when user is logined */
int show_prompt_without_user();         /* show prompt when user is not logined */
void cleaner();                         /* clean the temp file */
void process_input();                   /* process user input */
void get_register_input(register_message_t *);/* show register prompt and get input */
void get_login_input(login_message_t *); /* show login prompt and get input */
void init_logged_in_user(login_message_t *);/* init logged_in_user and user_reciver */
/**************************************************************************/
/* main function */

int main(int argc,char **argv) {
    init();
    listen_loop();
    return 0;
}


/*************************************************************************/
/* fatal error, clien will termina */
void fatalError(char *prompt) {
    perror(prompt);
    exit(-1/* Failure */);
}

/*************************************************************************/
/* init the client*/

void init() {
    /* init the configuration */
    init_config();

    /* init sender */
    init_sender();
    
    /* init temp reciver */
    //init_temp_reciver();
    sprintf(client_temp_name,"Client%d",getpid());
    printf("init over\n");
}


/*************************************************************************/
/* init the config */
void init_config() {
    int res;
    config = (config_t *)malloc(sizeof(config_t));
    
    if (config == NULL) fatalError("init_config:malloc config failure");

    res = config_parse(CONFIGURATIONFILE_PATH,config);

    if (res != 0) fatalError("init_config:parse_cofnig");
}

/**************************************************************************/
/* init the messenger for server */
void init_sender() {
    int res;

    reg_sender = (messenger_t*)malloc(sizeof(messenger_t));
    login_sender = (messenger_t*)malloc(sizeof(messenger_t));
    msg_sender = (messenger_t*)malloc(sizeof(messenger_t));

    if (reg_sender == NULL || login_sender == NULL || msg_sender == NULL)
        fatalError("init_sender:malloc failure");

    res = messenger_init(reg_sender,config->reg_path,Sender);
    if (res != 0) fatalError("init_sender:reg_sender init failure");

    res = messenger_init(login_sender,config->login_path,Sender);
    if (res != 0) fatalError("init_sender:login_sender init failure");

    res = messenger_init(msg_sender,config->sendmsg_path,Sender);
    if (res != 0) fatalError("init_sender:msg_sender init failure");
}

/*****************************************************************************/
/* init temp reciver for client */

void init_temp_reciver() {
    int res;
    char pathbuf[BUFF_SZ];
    
    //init the temp file path
    username2path(client_temp_name,pathbuf,Temp);
    printf("temp file:%s\n",pathbuf);
    //create temp file for reciver 
    res = mkfifo(pathbuf,0666);
    if (res != 0) fatalError("init_temp_reciver: create temp file failure");

    /* set signal signals */
    signal(SIGKILL, cleaner);
    signal(SIGINT, cleaner);
    signal(SIGTERM, cleaner);

    temp_reciver = (messenger_t*)malloc(sizeof(messenger_t));
    if (temp_reciver == NULL) fatalError("init_temp_reciver:malloc failure");


    res = messenger_init(temp_reciver,pathbuf, Reciver);
    if (res != 0) fatalError("init_temp_reciver: init failure");
}
/*******************************************************************************/
/* clean the temp file when client down */
void cleaner() {
    char path[BUFF_SZ];
    sprintf(path,"%s.temp%d",CLIENT_FILE_PREFIX,getpid());
    unlink(path);
}

/******************************************************************************/
/* listen loop  */

void listen_loop() {
    printf("listen_loop\n");
    int input;
    while(1) {
        /*if user logined, show user information, check user_reciver */
        if (logged_in_user == NULL)
            input = show_prompt_without_user();
        else 
            input = show_prompt_with_user();
        process_input(input);     
    }
}

/**************************************************************************/
/* show the prompt when user is logined. */
int show_prompt_with_user() {
    int input;
    
    printf("=====================================\n");
    printf("1. Send Message\n");
    printf("2. Recive Message\n");
    printf("=====================================\n");

    scanf("%d",&input);
    
    if(input == 1) return SendMsg;
    else if (input == 2) return ReciveMsg;
    else return DoNothing;
}

/*******************************************************************************/
/* show the prompt when user is not logined */
int show_prompt_without_user() {
    int input;
    printf("=====================================\n");
    printf("1. Register\n");
    printf("2. Login\n");
    printf("=====================================\n");

    scanf("%d", &input);

    if (input == 1) return Register;
    else if (input == 2) return Login;
    else return DoNothing;
}

/*****************************************************************************/
/* process the input */
void process_input(int input) {
    switch(input) {
        case SendMsg:
            send_msg();
            break;
        case ReciveMsg:
            recive_msg();
            break;
        case Register:
            register_user();
            break;
        case Login:
            login_user();
            break;
        case DoNothing:
        default:
            break;
    }
}

/*****************************************************************************/
/* register user */

void register_user() {
    register_message_t reg_msgbuf;
    chat_message_t reply_msgbuf;
    int reply;
    int res;

    //show the register prompt and get the input
    get_register_input(&reg_msgbuf); 
    
    strcpy(reg_msgbuf.temp_reciver ,client_temp_name);

    //send reg msg
    res = messenger_send(reg_sender,(void*)&reg_msgbuf,sizeof(register_message_t));
    
    printf("send res = %d\n",res);

    if (temp_reciver == NULL) init_temp_reciver();
    //recive msg
    res = messenger_recive(temp_reciver,(void *)&reply_msgbuf,sizeof(chat_message_t));
    printf("recive res = %d\n",res);

    //parse reply
    reply = parse_server_reply(&reply_msgbuf);
    switch(reply) {
        case SuccessReply:
              printf("Register Success\n");
              break;
        case WrongUserName:
              printf("Username is used or have wrong character\n"
                      "Please use another username.\n");
              break;
        case ServerError:
              printf("Server error, please check server.\n");
              break;
        default:
              printf("Can not parse server reply\n");
              break;
    }
}

/*****************************************************************************/
/* login user */

void login_user() {
    login_message_t login_msgbuf;
    chat_message_t reply_msgbuf;
    int reply;
    //show login prompt and get the input
    get_login_input(&login_msgbuf);

    //send login msg
    messenger_send(login_sender,(void*)&login_msgbuf,sizeof(login_message_t));

    //reciver reply msg
    messenger_recive(temp_reciver,(void *)&reply_msgbuf,sizeof(chat_message_t));

    reply = parse_server_reply(&reply_msgbuf);
    switch(reply) {
        case SuccessReply:
            printf("Login Success\n");
            init_logged_in_user(&login_msgbuf);
            break;
        case NoUser:
            printf("Wrong User, please register first.\n");
            break;
        case WrongPasswd:
            printf("Password is wrond, please try again\n");
            break;
        case ServerError:
            printf("Server error, please check server.\n");
            break;
        default:
            printf("Can not parse server reply\n");
            break;
    }
}

/*****************************************************************************/
/* recive msg */

void recive_msg() {

}

/*******************************************************************************/
/* send message */

void send_msg() {

}

/*****************************************************************************/
/* get the register input */
void get_register_input(register_message_t *msgbuf) {
    char passbuf[MESSAGE_BUFFER_SZ];

    /* show the prompt and get input */
    while(1) {
        printf("Please input username:\n");
        scanf("%s",msgbuf->username);
        if (strcmp(msgbuf->username,SERVER_NAME) != 0) break;
        printf("Wrong username, please choice another.\n");
    }
    
    while(1) {
        printf("Please input password:\n");
        scanf("%s",passbuf);
        printf("Please input password again:\n");
        scanf("%s",msgbuf->passwd);
        //check the password     
        if (strcmp(passbuf,msgbuf->passwd) == 0) break;
        printf("The two password don't match\n");
    }
}

/*********************************************************************************/
/* show login prompt and get the input */

void get_login_input(login_message_t *msgbuf) {


}

/********************************************************************************/
/* init logged_in_user and user reciver */
void init_logged_in_user(login_message_t *userbuf) {

}
