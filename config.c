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
#define MAX_LINES 4
/************************************************************************/
/* some function declaration */

void pass1(int fd,int *lineStart); /* get pre line offset */
int pass2(int fd,int *lineStart,
        char *reg_path,char *login_path,char *sendmsg_path,int *max_user,
        int line_max_len); /* parse config */


/**************************************************************************/
/* read config file and parse */

int config_parse(char *config_path, char *reg_path,
        char *login_path, char *sendmsg_path, int *max_user,int line_max_len) {
    
    int res,fd;
    int lineStart[MAX_LINES + 1]; /* the config file we only read the first four line */
    lineStart[0] = 0;

    /* read the configuration file */
    if ((fd = open(config_path,O_RDONLY)) == -1) {
        return -1;/* Failure */
    }

    pass1(fd,lineStart);
    res = pass2(fd,lineStart,reg_path,login_path,sendmsg_path,max_user,line_max_len);

    return res;
}



/***************************************************************************/
/* first parse, get pre line offset */
void pass1(int fd,int *lineStart) {
    int charRead,i;
    char buffer[BUFF_SZ];
    int fileOffset = 0;
    int lineCount = 1;
    
    while(1) {
        charRead = read(fd,buffer,BUFF_SZ);
        if (charRead == 0) break;
        if (charRead == -1) {
            perror("config:pass1:read failure"); exit(-1);
        }
        
        //process
        for(i = 0;i < charRead;i++) {
            fileOffset++;
            if (buffer[i] == '\n') 
                lineStart[lineCount++] = fileOffset;
        }

        if (lineCount == MAX_LINES + 1) break;
    }
    while(lineCount <= MAX_LINES)
        lineStart[lineCount++] = fileOffset;
}

/****************************************************************************/
/* second parse, use lineStar to parse preline */

int pass2(int fd,int *lineStart,
        char *reg_path,char *login_path,char *sendmsg_path,int *max_user,
        int line_max_len) {

    int i, charRead;
    char *buffer = (char*)malloc(sizeof(char)*(line_max_len + 2));
    int read_len;
    if (buffer == NULL) return -1; /*malloc faliure*/

    for(i = 0;i < MAX_LINES;i++) {
        //read
        read_len = lineStart[i+1] - lineStart[i];
        if (read_len == 0) continue;
        
        if (read_len - 2 > line_max_len) return -1;

        lseek(fd,lineStart[i],SEEK_SET);
        charRead = read(fd,buffer,read_len);
        //fix some error
        if (charRead < 0) return -1; //parse error
        if (charRead != read_len) return -1;
        if (charRead < 2) return -1;
        if (buffer[1] != ':') return -1;
        
        

        if (buffer[charRead-1] == '\n')
            buffer[charRead-1] = '\0'; //replace '\n' to '\0'
//        printf("%s",buffer+2);
        switch(buffer[0]) {
            case 'L':
                strcpy(login_path,buffer+2);
                break;
            case 'R':
                strcpy(reg_path,buffer+2);
                break;
            case 'S':
                strcpy(sendmsg_path,buffer+2);
                break;
            case 'M':
                if(max_user != NULL) *max_user = atoi(buffer+2);
                break;
            default:
                return -1;
        }
    }

    free(buffer);
    return 0; /*parse success */

}
