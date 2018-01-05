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
    Login,Register,SendMsg,ReciveMsg,DoNothing,Exit
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
void create_temp_file();                /* create temp file for temp reciver */
void get_sendmsg_input();               /* show the prompt and get input */





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
    create_temp_file();

    user_reciver = (messenger_t*)malloc(sizeof(messenger_t));
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
/* just create temp file */
void create_temp_file() {
    int res;
    char pathbuf[BUFF_SZ];
    
    //init client temp file name
    sprintf(client_temp_name,"Client%d",getpid());
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
}

/********************************************************************************/
/* init temp reciver for client */

void init_temp_reciver() {
    int res;
    char pathbuf[BUFF_SZ];
    //init the temp path
    username2path(client_temp_name,pathbuf,Temp);

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
    else if (input == 0) return Exit;
    else return DoNothing;
}

/*******************************************************************************/
/* show the prompt when user is not logined */
int show_prompt_without_user() {
    int input;
    printf("=====================================\n");
    printf("1. Register\n");
    printf("2. Login\n");
    printf("0. Exit\n");
    printf("=====================================\n");

    scanf("%d", &input);

    if (input == 1) return Register;
    else if (input == 2) return Login;
    else if (input == 0) return Exit;
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
        case Exit:
            exit(0);
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
   // printf("reg msg-t size is %lu\n",sizeof(register_message_t));
    res = messenger_send(reg_sender,(void*)&reg_msgbuf,sizeof(register_message_t));
    
  //  printf("send res = %d\n",res);

    init_temp_reciver();
    //recive msg
    res = messenger_recive(temp_reciver,(void *)&reply_msgbuf,sizeof(chat_message_t));
//    printf("recive res = %d\n",res);

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

    messenger_destory(temp_reciver);
}

/*****************************************************************************/
/* login user */

void login_user() {
    login_message_t login_msgbuf;
    chat_message_t reply_msgbuf;
    int reply,res;
    //show login prompt and get the input
    get_login_input(&login_msgbuf);
    
    strcpy(login_msgbuf.temp_reciver,client_temp_name);

    //send login msg
    messenger_send(login_sender,(void*)&login_msgbuf,sizeof(login_message_t));
    
    init_temp_reciver();


    //reciver reply msg
    res = messenger_recive(temp_reciver,(void *)&reply_msgbuf,sizeof(chat_message_t));
    //printf("res is %d\n",res);

    //message_show(&reply_msgbuf);

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
        case UserIsLoggedIn:
            printf("User is logged in.\n");
            break;
        case UserIsTooMuch:
            printf("Server have too much logged in users.\n");
            break;
        default:
            printf("Can not parse server reply, reply is %d\n",reply);
            break;
    }
    messenger_destory(temp_reciver);
}

/*****************************************************************************/
/* recive msg */

void recive_msg() {
    int res,i;
    chat_message_t msgbuf;
    int read = 0;
    for(i = 0,res = 0;i < 100 && read != sizeof(chat_message_t);i++) {
        res = messenger_recive(user_reciver,&msgbuf + res,sizeof(chat_message_t));
        read += res;
        if (read != 0) {
     //       printf("read is %d\n",read);
            i = 0;
        }
    }
    if (res > 0) {
        printf("======================================\n");
        printf("Sender:%s\n",msgbuf.sender);
        printf("Reciver:%s\n",msgbuf.reciver);
        printf("Message:%s\n",msgbuf.message);
        printf("======================================\n");
    }
}

/*******************************************************************************/
/* send message */

void send_msg() {
//    printf("send_msg\n");
    chat_message_t msgbuf;
    int res = 0;

    //show send_msg prompt and get the input
    get_sendmsg_input(&msgbuf);
    strcpy(msgbuf.sender,logged_in_user->username);
    
    res = messenger_send(msg_sender,&msgbuf,sizeof(chat_message_t));
    if (res <= 0) {
        perror("send_msg failure"); return;
    }

    //because the user_reciver become not block.
    res = 0;
    while(res <= 0) {
        res = messenger_recive(user_reciver,&msgbuf,sizeof(chat_message_t));
    }
    
  //  printf("Server reply\n");
   // message_show(&msgbuf);

    res = parse_server_reply(&msgbuf);
    switch(res) {
        //in fact, WrongSender will not be return in our application.
        case WrongSender:
            printf("sender is wrong, maybe client have some bug\n");
            break;
        case UserIsNotLoggedIn:
            printf("You are not login!\n");
            break;
        case WrongReciver:
            printf("Reciver is not exist.\n");
            break;
        case SuccessReply:
            printf("Send message success.\n");
            break;
        case ParseError:
            printf("Parse reply error.\n");
            message_show(&msgbuf);
            break;
    }
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
    printf("Please input username\n");
    scanf("%s",msgbuf->username);
    printf("Please input password\n");
    scanf("%s",msgbuf->passwd);
}

/********************************************************************************/
/* init logged_in_user and user reciver */
void init_logged_in_user(login_message_t *userbuf) {
    char pathbuf[BUFF_SZ];
    int res;

    user_reciver = (messenger_t*)malloc(sizeof(messenger_t));
    if (user_reciver == NULL) fatalError("init_logged_in_user:malloc user_reciver");

    username2path(userbuf->username,pathbuf,Client);
    
    res = messenger_init(user_reciver,pathbuf,Reciver);
    //printf("messenger_init res is %d\n",res);
    if (res != 0) fatalError("init_logged_in_user");

    logged_in_user = (user_t*)malloc(sizeof(user_t));
    if (logged_in_user == NULL) fatalError("init_logged_in_user\n");

    strcpy(logged_in_user->username,userbuf->username);
    strcpy(logged_in_user->passwd,userbuf->passwd);
}

/*********************************************************************************/
/* show send msg prompt and get the input */
void get_sendmsg_input(chat_message_t *msgbuf) {
    printf("===========================================\n");
    printf("Reciver:\n");
    scanf("%s",msgbuf->reciver);
    printf("Message:\n");
    scanf("%s",msgbuf->message);
    printf("===========================================\n");
}
