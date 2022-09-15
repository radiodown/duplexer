#include "syshead.h"
#include "basic.h"
#include "logger.h"
#include "version.h"
#include "options.h"

extern int log_level;

void show_version(){
    logger(LOG_INFO, "%s", PACKAGE_STRING);
    logger(LOG_INFO, "%s", PACKAGE_COPYRIGHT);
    logger(LOG_INFO, "Updated : %s", PACKAGE_LAST_UPDATE);
    logger(LOG_INFO, "Developed by : %s", PACKAGE_BUGREPORT);
}

void usage(){
    printf("\n");
    printf("General Options:\n");
    printf("--config file   : Read configuration options from file.\n");
    printf("--help          : Show options.\n");
    printf("--version       : Show copyright and version information.\n");
    printf("\n");
}

int add_config(char *p[], struct options* o){
    if(strcmp("config", p[0]) == 0){
        o->config_file = (char *)malloc(strlen(p[1]) * sizeof(char) + 1);
        CLEAR(*o->config_file);
        strcpy(o->config_file,p[1]);
        logger(LOG_DEBUG,"Config file : %s",o->config_file);
        return 0;
    } else if (strcmp("help", p[0]) == 0) {
        usage();
    } else if (strcmp("version", p[0]) == 0) {
        printf("%s\n%s\n%s\n%s\n", PACKAGE_STRING,PACKAGE_COPYRIGHT,PACKAGE_LAST_UPDATE,PACKAGE_BUGREPORT);
    }
    return 1;
}

void init_options(struct options* o){
    log_level = 1;
    logger(LOG_DEBUG, "START INIT OPTIONS");

    o = malloc(sizeof(struct options));
    if(o == NULL){
         logger(LOG_DEBUG, "FAILED TO init");
    }
    CLEAR(*o);
    o->duplex = 0;
    o->layer_count = 0;

    logger(LOG_DEBUG, "END INIT OPTIONS");
}

int parse_argv(struct options* o, int argc, char* argv[]){
    int i,j,ret;

    /* usage message */
    if (argc <= 1) {
        usage();
        return 1;
    }

    /* config filename specified only */
    if (argc == 2 && strncmp(argv[1], "--", 2)) {
        o->config_file = malloc(strlen(argv[1]) + 1);
        strncpy(o->config_file,argv[1],strlen(argv[1]));
        ret = 0;
        logger(LOG_DEBUG,"Config file : %s",o->config_file);
    } else {
        /* parse command line */
        for (i = 1; i < argc; ++i)
        {
            char *p[MAX_PARAMS];
            CLEAR(p);
            p[0] = argv[i];
            if (strncmp(p[0], "--", 2)) {
                logger(LOG_INFO, "Cannot parse option : %s", p[0]);
                return 1;
            }
            else {
                p[0] += 2;
            }

            for (j = 1; j < MAX_PARAMS; ++j) {
                if (i + j < argc) {
                    char *arg = argv[i + j];
                    if (strncmp(arg, "--", 2)) {
                        p[j] = arg;
                    } else {
                        break;
                    }
                }
            }
            i += j - 1;
            ret = add_config(p, o);
        }
    }
    
    return ret;
}

int parse_config(struct options* o, char* p[]){
    if(strcmp(p[0], "duplex") == 0){
        if(strcmp(p[1],"on") == 0){
            o->duplex = 1;
        }else{
            o->duplex = 0;
        }
    }else if(strcmp(p[0], "mode") == 0){
        if(strcmp(p[1],"master") == 0){
            o->mode = 1;
        }else if(strcmp(p[1],"slave") == 0){
            o->mode = 0;
        }else{
            o->mode = 1;
        }
    }else if(strcmp(p[0], "gw") == 0){
        if(strcmp(p[0],"gw") == 0){
            o->l[o->layer_count].gateway = malloc(strlen(p[1]) +1);
            CLEAR(*o->l[o->layer_count].gateway);
            strcpy(o->l[o->layer_count].gateway, p[1]);
        } 
        if(strcmp(p[2],"vip") == 0){
            o->l[o->layer_count].vip = malloc(strlen(p[3])+1);
            CLEAR(*o->l[o->layer_count].vip);
            strcpy(o->l[o->layer_count].vip, p[3]);
        } 
        if(strcmp(p[4],"if") == 0){
            o->l[o->layer_count].interface = malloc(strlen(p[5])+1);
            CLEAR(*o->l[o->layer_count].interface);
            strcpy(o->l[o->layer_count].interface, p[5]);
        } 
        if(strcmp(p[6],"count") == 0){
            o->l[o->layer_count].count = atoi(p[7]);
        } 
        if(strcmp(p[8],"dup") == 0){
            o->l[o->layer_count].dup = malloc(strlen(p[9]) + 1);
            CLEAR(*o->l[o->layer_count].dup);
            strcpy(o->l[o->layer_count].dup, p[9]);
        }
        o->layer_count++;
        
    }else if(strcmp(p[0], "direct") == 0 ){
        if(strcmp(p[1], "off") == 0){
            o->direct = 0;
        }else{
            o->direct = 1;
           
            o->direct_ip = malloc(strlen(p[1])+1);
            CLEAR(*o->direct_ip);
            strcpy(o->direct_ip, p[1]);
            
            if(strcmp(p[2],"if") == 0){
                o->direct_interface = malloc(strlen(p[3])+1);
                CLEAR(*o->direct_interface);
                strcpy(o->direct_interface, p[3]);
            } 
            if(strcmp(p[4],"port") == 0){
                o->direct_port = atoi(p[5]);
            }
        }
    }else if(strcmp(p[0], "log") == 0 ){
        o->log_file = malloc(strlen(p[1])+1);
        CLEAR(*o->log_file);
        strcpy(o->log_file, p[1]);
    }else if(strcmp(p[0], "syslog") == 0 ){
        if(strcmp(p[1],"on") == 0){
            o->syslog = 1;
        }else{
            o->syslog = 0;
        }
    }else if(strcmp(p[0], "log-level") == 0 ){
        o->log_level = atoi(p[1]);
        log_level = o->log_level;
    }else if(strcmp(p[0], "management") == 0 ){
        o->management_port = atoi(p[1]);
    }else{
        logger(LOG_INFO,"Unrecognized Options in Config, Exit Program. [%s]", p[0]);
        return 1;
    }
    return 0;
}

int parse_line(char* line, char* p[]){
    //logger(LOG_DEBUG, "start parse line");
    //logger(LOG_DEBUG, "line : [%s]", line);
    char *ptr = strtok(line, " ");
    int i = 0;

    while (ptr != NULL){
        //logger(LOG_DEBUG,"ptr : %s", ptr);
        /* pass spacebar */
        if(strcmp(ptr," ") == 0){ 
            ptr = strtok(NULL, " ");
            continue;
        }
        p[i] = malloc(strlen(ptr)+1);
        CLEAR(*p[i]);
        strcpy(p[i],ptr);
        //logger(LOG_DEBUG,"p[%d] : %s", i, p[i]);
        ptr = strtok(NULL, " ");
        i++;
    }
    return i;
}

void parse_free(char* p[], int size){
    //logger(LOG_DEBUG, "%d", size);
    for(int i = 0; i < size-1; i++){
        free(p[i]);
    }
}

int read_config_file(struct options* o){
    char line[1024] = {0,};
    char* p[32];

    FILE* fp = fopen(o->config_file, "r");
    if(fp == NULL){
        logger(LOG_INFO,"Error Opening config file");
        return 1;
    }

    /* read file line by */
    while(fgets(line, sizeof(line), fp)){
        /* pass remark and newline */
        if(strstr(line,"#") == NULL  && strcmp(line, "\n") != 0){
            /* remove newline */
            line[strlen(line) - 1] = '\0';
            /* parse one line */
            int size = parse_line(line, p);
            //logger(LOG_TRACE,"p : %s, %s", p[0], p[1] );
            /* copy to Struct options */
            if(parse_config(o, p)){
                return 1;
            }
            /* free memory */
            parse_free(p,size);
        }
    }

    return 0;
}

int show_options(struct options* o){
    logger(LOG_DEBUG,"duplex :%s",o->duplex == 1 ? "on" : "off" );
    if(o->duplex == 1){
        logger(LOG_DEBUG,"mode   :%s",o->duplex == 1 ? "master" : "slave" );
        for (int i = 0;i < o->layer_count;i++){
            logger(LOG_DEBUG,"layer [%d]", i);
            logger(LOG_DEBUG,"   Gateway    :%s", o->l[i].gateway);
            logger(LOG_DEBUG,"   Virtual IP :%s", o->l[i].vip);
            logger(LOG_DEBUG,"   interface  :%s", o->l[i].interface);
            logger(LOG_DEBUG,"   Fail Count :%d", o->l[i].count);
            logger(LOG_DEBUG,"   duplex IP  :%s", o->l[i].dup);
        }
        logger(LOG_DEBUG,"HA     :%s",o->direct == 1 ? "on" : "off" );
        if(o->direct){
            logger(LOG_DEBUG,"HA IP  :%s",o->direct_ip);
            logger(LOG_DEBUG,"HA IF  :%s",o->direct_interface);
            logger(LOG_DEBUG,"HA PORT:%d",o->direct_port);
        }
        logger(LOG_DEBUG,"log    :%s",o->log_file);
        logger(LOG_DEBUG,"Syslog :%s",o->syslog ==1? "on" : "off" );
        logger(LOG_DEBUG,"Log lvl:%d",o->log_level);
        logger(LOG_DEBUG,"MG PORT:%d",o->management_port);
        return 0;
    }else{
        logger(LOG_INFO,"Duplexer set off, Exit Program");
        return 1;
    }
}


 
