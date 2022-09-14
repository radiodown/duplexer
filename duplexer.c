#include "syshead.h"
#include "duplexer.h"
#include "basic.h"
#include "logger.h"
#include "master.h"
#include "slave.h"

int main(int argc, char* argv[]){
    context c;

    CLEAR(c);

    init_options(&c.o);

    if(parse_argv(&c.o, argc, argv)){
        return 1;
    }

    show_version();

    if(read_config_file(&c.o)){
        return 1;
    }

    if(show_options(&c.o)){
        return 0;
    }

    if(c.o.mode == 1){
        /* Enter master mode */
        mode_master(&c);
    }else{
        /* Enter slave mode */
        mode_slave(&c);
    }

    return 0;
}