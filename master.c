#include "master.h"
#include "syshead.h"
#include "logger.h"
#include "ping.h"

int check_vip(){

}

int install_vip(){

}

int waiting_slave(){

}


/* HA Connected */
//  GW 연결           DUP 연결         HA 연결
//   실패               실패            성공      VIP다운, Your plane
//   실패               성공            성공      VIP다운, Your plane   
//   성공               실패            성공      VIP업, My Plane, Dup점검 로그
//   성공               성공            성공      VIP업, My Plane

/* HA Disconnected */
//  GW 연결           DUP 연결         HA 연결
//   실패               실패            싪패      VIP다운
//   실패               성공            싪패      VIP다운
//   성공               실패            싪패      VIP업, Dup점검 로그
//   성공               성공            싪패      VIP업

int mode_master(context* c){
    logger(LOG_INFO, "Entering Master Mode");
    ping_main("10.0.1.50", 3);

    while(1){
        /* Checking HA */
        /* Using HA */
        if(c->o.direct == 1){
            if(ping_main(c->o.direct_ip, 3)){
                c->s[0].ha_status = 1;
                c->s[1].ha_status = 1;
            }else {
                c->s[0].ha_status = 0;
                c->s[1].ha_status = 0;
            }
        } 
        /* Not Using HA */
        else {
            c->s[0].ha_status = 1;
            c->s[1].ha_status = 1;
        }
            
        /* Check GW, Opponent(dup) */
        for (int i = 0; i < c->o.layer_count; i++){
            if(ping_main(c->o.l[i].gateway, c->o.l[i].count)){
                c->s[i].gw_status = 1;
            }else{
                c->s[i].gw_status = 0;
            }
            
            if(ping_main(c->o.l[i].dup, c->o.l[i].count)){
                c->s[i].dup_status = 1;
            }else{
                c->s[i].dup_status = 0;
            }
        }

        
        for (int i = 0; i < c->o.layer_count; i++){
            logger(LOG_DEBUG, "%d, %d", c->s[i].gw_status, c->s[i].dup_status);
            /* HA  Disonnected */
            if(c->s[i].ha_status){
                logger(LOG_DEBUG, "HA DISCONNECTED");
                /* GW, DUP Failed */
                if(c->s[i].gw_status && c->s[i].dup_status ){
                    logger(LOG_DEBUG, "GW, DUP FAILED");
                    /* VIP down */

                }
                /* GW, DUP Success */
                else if (c->s[i].gw_status == 0 && c->s[i].dup_status == 0) {
                    /* VIP up */
                    logger(LOG_DEBUG, "GW, DUP Success");

                }
                else {
                    /* GW Fail, Dup Success */
                    if (c->s[i].gw_status)
                    {
                        /* VIP down */
                        logger(LOG_DEBUG, "GW Failed, DUP Success");
                    }
                    /* GW Success, Dup Fail */
                    else{
                        logger(LOG_DEBUG, "GW Success, DUP FAILED");
                        /* VIP up, check network log*/
                    }
                }
            }
            /* HA  Connected */
            else{
                logger(LOG_DEBUG, "HA CONNECTED");
                 /* GW, DUP Failed */
                if(c->s[i].gw_status && c->s[i].dup_status){
                    /* VIP down */
                    logger(LOG_DEBUG, "GW, DUP FAILED");
                }
                /* GW, DUP Success */
                else if (c->s[i].gw_status == 0 && c->s[i].dup_status == 0) {
                    /* VIP up */
                    logger(LOG_DEBUG, "GW, DUP Success");
                }
                else {
                    /* GW Fail, Dup Success */
                    if (c->s[i].gw_status)
                    {
                        /* VIP down */
                        logger(LOG_DEBUG, "GW Failed, DUP Success");
                    }
                    /* GW Success, Dup Fail */
                    else{
                        /* VIP up, check network log*/
                        logger(LOG_DEBUG, "GW Success, DUP FAILED");
                    }
                }
            }
        }

        sleep(1);
    }
    return 0;
}