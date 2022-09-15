#ifndef OPTIONS_H
#define OPTIONS_H

/*
 * Maximum number of parameters associated with an option,
 * including the option name itself.
 */
#define MAX_PARAMS 16

/* Connecting gateway layer */
struct layer {
    /* Gateway address */
    char* gateway; 
    /* To Announce Virtual IP */
    char* vip;
    /* Vip's netmask */
    char* netmask;
    /* Interface*/
    char* interface;
    /* Fail count */
    int count;
    /* Opponent IP address */
    char* dup;
};


struct options {
    /* config file name */
    char* config_file;
    /* 1: on 0: off */
    int duplex; 
    /* master: 1 or slave: 0 */
    int mode;
    /* array of layer */
    struct layer l[2];
    /* count of layer */
    int layer_count;
    /* 1: use 0: not use, using direct cable */
    int direct;
    /* directed connected Opponent's ip */
    char* direct_ip;
    /* directed connected interface */
    char* direct_interface;
    /* directed connected port */
    int direct_port;
    /* log file name */
    char* log_file;
    /* log level : 1 2 3 4 5 6 7, 1: INFO, 2: debug, 4: trace  */
    int log_level;
    /* 1: on 0: off */
    int syslog;
    /* telnet management port, 0: not using */
    int management_port;
};

int parse_argv(struct options* o, int argc, char* argv[]);
void init_options(struct options* o);
int read_config_file(struct options* o);
int show_options(struct options* o);
void show_version();

#endif /* OPTIONS_H */