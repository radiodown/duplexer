#include "master.h"
#include "syshead.h"
#include "logger.h"
#include "httpserver.h"
#include "ping.h"

int port;

void* t_function(void* data) {
    http_server(port, (int*)data);
    return NULL;
}

/* HA Connected */
// 마스터 설정을 따라감

/* HA Disconnected */
//  GW 연결           DUP 연결         HA 연결
//   실패               실패            싪패      VIP다운
//   실패               성공            싪패      VIP다운
//   성공               실패            싪패      VIP업, Dup점검 로그
//   성공               성공            싪패      VIP다운

void mode_slave(context* c){
    logger(LOG_INFO, "Entering Master Mode");

    pthread_t pthread;
    int thr_id;
    int result;
    port = c->o.direct_port;
    /* 1: Master's Plane, 0: Slave's Plane */
    int pilot = 0;
    
    if(c->o.direct == 1){
        thr_id = pthread_create(&pthread, NULL, t_function, (void*)&pilot);
        if(thr_id < 0) {
          perror("pthread0 create error");
          exit(EXIT_FAILURE);
        }
    }

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

    pthread_join(pthread, NULL);
}