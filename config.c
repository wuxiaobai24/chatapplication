#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"

/* just for pass1, so line length can longer than BUFF_SZ */
#define BUFF_SZ 100
/************************************************************************/
/* some function declaration */

/* get pre line offset */
void pass1(int fd,int *lineStart); 
/* process buffer for parse line offset*/
void processLine(char *buffer,int charRead,int *lineStart);
/* parse config */
int pass2(int fd,int *lineStart,config_t *config); 
/* parse line */
int parseLine(char *buffer,int charRead,config_t *config);

/***********************************************************************/
/* local value */
static int fileOffset;
static int lineCount;

/**************************************************************************/
/* read config file and parse */

int config_parse(char *config_path, config_t *config) {
    
    //check the pointer is not NULL
    if (config == NULL || config_path == NULL) return CONFIG_FAILURE;
    
    int res,fd;
    int lineStart[CONFIG_SIZE + 1]; /* the config file we only read the first four line */
    lineStart[0] = 0;

    printf("config_path:%s\n",config_path);
    
    /* read the configuration file */
    if ((fd = open(config_path,O_RDONLY)) == -1) {
        return -1;/* Failure */
    }

    pass1(fd,lineStart);
    res = pass2(fd,lineStart,config);

    return res;
}



/***************************************************************************/
/* first parse, get pre line offset */

void pass1(int fd,int *lineStart) {
    int charRead,i;
    char buffer[BUFF_SZ];
    fileOffset = 0;
    lineCount = 1;
    
    while(1) {
        charRead = read(fd,buffer,BUFF_SZ);
        if (charRead == 0) break;
        if (charRead == -1) {
            perror("config:pass1:read failure"); exit(-1);
        }
        
        //process
        processLine(buffer,charRead,lineStart);
        if (lineCount == CONFIG_SIZE + 1) break;
    }
    lineStart[lineCount++] = fileOffset;
}


/***************************************************************************/
/* process buffer for parse line offset */

void processLine(char *buffer,int charRead,int *lineStart) {
    int i;
    for(i = 0;i < charRead;i++) {
        fileOffset++;
        if (buffer[i] == '\n') 
            lineStart[lineCount++] = fileOffset;
    }
}



/****************************************************************************/
/* second parse, use lineStar to parse preline */

int pass2(int fd,int *lineStart,config_t *config) {

    int i, charRead,res;
    char *buffer = (char*)malloc(sizeof(char)*(CONFIG_BUFFER_SZ + 2));
    int read_len;
    if (buffer == NULL) return -1; /*malloc faliure*/

    for(i = 0;i <= CONFIG_SIZE;i++) {
        //read
        read_len = lineStart[i+1] - lineStart[i];
        if (read_len == 0) break; //ending
        
        if (read_len - 2 > CONFIG_BUFFER_SZ) return -1;
        
        lseek(fd,lineStart[i],SEEK_SET);
        charRead = read(fd,buffer,read_len);
        
        if (charRead < 0) return -1; //parse error
        if (charRead != read_len) return -1;
        

        res = parseLine(buffer,charRead,config);
        if (res != 0) return res;
    }
    free(buffer);
    return 0; /*parse success */

}

/*******************************************************************************/
/* parse line */
int parseLine(char *buffer,int charRead,config_t *config) {
    if (charRead < 2) return -1;
    if (buffer[1] != ':') return -1;

    if (buffer[charRead-1] == '\n')
        buffer[charRead-1] = '\0'; //replace '\n' to '\0'
    switch(buffer[0]) {
        case 'L':
            strcpy(config->login_path,buffer+2);
            break;
        case 'R':
            strcpy(config->reg_path,buffer+2);
            break;
        case 'S':
            strcpy(config->sendmsg_path,buffer+2);
            break;
        case 'M':
            config->max_user = atoi(buffer+2);
            break;
        default:
            return -1;
    }
    return 0;
}
