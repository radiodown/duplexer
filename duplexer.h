#ifndef DUPLEXER_H
#define DUPLEXER_H

#include "options.h"

/* 0 : Connected 1 : Disconnected */
struct status{
    /* ping to gateway */
    int gw_status;
    /* ping to opponent duplexer through ethernet */ 
    int dup_status;
    /* ping to opponent duplexer through HA */
    int ha_status;
};

typedef struct _context {
    struct options o;
    struct status s[2];
}context;

#endif
