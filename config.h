#ifndef _CONFIG_H
#define _CONFIG_H

#define CONFIG_BUFFER_SZ 100

#define CONFIG_SIZE 4

/*the config struct only have CONFIG_SIZE items. */
typedef struct {
    char reg_path[CONFIG_BUFFER_SZ];
    char login_path[CONFIG_BUFFER_SZ];
    char sendmsg_path[CONFIG_BUFFER_SZ];
    int max_user;
} config_t;

#define CONFIG_SUCCESS 0
#define CONFIG_FAILURE -1

int config_parse(char *config_file,config_t *config);

#endif
