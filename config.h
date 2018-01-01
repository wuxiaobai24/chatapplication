#ifndef _CONFIG_H
#define _CONFIG_H



int config_parse(char *config_file, char *reg_path, 
        char *login_path, char *sendmsg_path, int *max_user,int line_max_len);

#endif
