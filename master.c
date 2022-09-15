#include "master.h"
#include "syshead.h"
#include "logger.h"
#include "ping.h"
#include "httpclient.h"
#include "basic.h"
#include "vip.h"

#define DUPLEXER_ALIVE "/duplexer/alive"
#define DUPLEXER_MYPLANE "/duplexer/myplane"
#define DUPLEXER_YOURPLANE "/duplexer/yourplane"

// int check_vip(){

// }

// int install_vip(){

// }

// int waiting_slave(){

// }


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

void mode_master(context* c){
    int alive_count = 0;
    int vip_status[2] = {0,}; // 1: up, 0: down

    logger(LOG_INFO, "Entering Master Mode");

    while(1){
        /* Checking HA */
        /* Using HA */
        if(c->o.direct == 1){
            /* 0: success 1: fail */
            if(ping_main(c->o.direct_ip, 3)){
                c->s[0].ha_status = 1;
                c->s[1].ha_status = 1;
            }else {
                c->s[0].ha_status = 0;
                c->s[1].ha_status = 0;
                if(alive_count < 3){
                    logger(LOG_DEBUG,"[ MASTER ]: is Alive?");
                    if(send_http(c->o.direct_port, c->o.direct_ip, DUPLEXER_ALIVE)){
                        logger(LOG_INFO, "Cannot Connect Opponent's Duplexer, waiting 3s.");
                        sleep(3);
                        alive_count++;
                        continue;
                    }else{
                        alive_count = 0;
                    }
                }else if(alive_count == 3){
                    logger(LOG_INFO, "Cannot Connect Opponent's Duplexer, Change Direct off.");
                    alive_count++;
                    c->s[0].ha_status = 1;
                    c->s[1].ha_status = 1;
                }else{
                    c->s[0].ha_status = 1;
                    c->s[1].ha_status = 1;
                } 
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
                    if(vip_status[i]){
                        if(down_vip(c->o.l[i].interface)){
                            logger(LOG_INFO,"Failed to down interface %s", c->o.l[i].interface);
                        }else{
                            vip_status[i] = 0;
                        }
                    }
                }
                /* GW, DUP Success */
                else if (c->s[i].gw_status == 0 && c->s[i].dup_status == 0) {
                    /* VIP up */
                    logger(LOG_DEBUG, "GW, DUP Success");
                    if(vip_status[i] == 0){
                        if(install_vip(c->o.l[i].interface, c->o.l[i].vip)){
                            logger(LOG_INFO,"Failed to install vip %s", c->o.l[i].interface);
                        }
                        usleep(500);
                        if(install_netmask(c->o.l[i].interface, c->o.l[i].netmask)){
                            logger(LOG_INFO,"Failed to install netmask %s", c->o.l[i].netmask);
                        }else{
                            logger(LOG_DEBUG,"Successfully installed vip");
                            vip_status[i] = 1;
                        }
                    }
                }
                else {
                    /* GW Fail, Dup Success */
                    if (c->s[i].gw_status) {
                        /* VIP down */
                        logger(LOG_DEBUG, "GW Failed, DUP Success");
                        if(vip_status[i]){
                            if(down_vip(c->o.l[i].interface)){
                                logger(LOG_INFO,"Failed to down interface %s", c->o.l[i].interface);
                            }else{
                                vip_status[i] = 0;
                            }
                        }
                    }
                    /* GW Success, Dup Fail */
                    else{
                        logger(LOG_DEBUG, "GW Success, DUP FAILED");
                        /* VIP up, check network log*/
                        if(vip_status[i] == 0){
                            if(install_vip(c->o.l[i].interface, c->o.l[i].vip)){
                                logger(LOG_INFO,"Failed to install vip %s", c->o.l[i].interface);
                            }
                            usleep(500);
                            if(install_netmask(c->o.l[i].interface, c->o.l[i].netmask)){
                                logger(LOG_INFO,"Failed to install netmask %s", c->o.l[i].netmask);
                            }else{
                                vip_status[i] = 1;
                                logger(LOG_INFO, "Slave is Dead, Check network");
                            }
                        }
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
                    if(vip_status[i]){
                        if(down_vip(c->o.l[i].interface)){
                            logger(LOG_INFO,"Failed to down interface %s", c->o.l[i].interface);
                        }else{
                            vip_status[i] = 0;
                        }
                    }
                    if(send_http(c->o.direct_port, c->o.direct_ip, DUPLEXER_YOURPLANE)){
                        logger(LOG_INFO,"Failed to Send HA status");
                    }
                }
                /* GW, DUP Success */
                else if (c->s[i].gw_status == 0 && c->s[i].dup_status == 0) {
                    /* VIP up */
                    logger(LOG_DEBUG, "GW, DUP Success");
                    if(vip_status[i] == 0){
                        if(install_vip(c->o.l[i].interface, c->o.l[i].vip)){
                            logger(LOG_INFO,"Failed to install vip %s", c->o.l[i].interface);
                        }
                        usleep(500);   
                        if(install_netmask(c->o.l[i].interface, c->o.l[i].netmask)){
                            logger(LOG_INFO,"Failed to install netmask %s", c->o.l[i].netmask);
                        }else{
                            logger(LOG_DEBUG,"Successfully installed vip");
                            vip_status[i] = 1;
                        } 
                    }
                    if(send_http(c->o.direct_port, c->o.direct_ip, DUPLEXER_MYPLANE)){
                        logger(LOG_INFO,"Failed to Send HA status");
                    }
                } else {
                    /* GW Fail, Dup Success */
                    if (c->s[i].gw_status) {
                        /* VIP down */
                        logger(LOG_DEBUG, "GW Failed, DUP Success");
                        if(vip_status[i]){
                            if(down_vip(c->o.l[i].interface)){
                                logger(LOG_INFO,"Failed to down interface %s", c->o.l[i].interface);
                            }else{
                                vip_status[i] = 0;
                            }
                        }
                        if(send_http(c->o.direct_port, c->o.direct_ip, DUPLEXER_YOURPLANE)){
                            logger(LOG_INFO,"Failed to Send HA status");
                        }
                    }
                    /* GW Success, Dup Fail */
                    else{
                        /* VIP up, check network log*/
                        logger(LOG_DEBUG, "GW Success, DUP FAILED");
                        if(vip_status[i] == 0){
                            if(install_vip(c->o.l[i].interface, c->o.l[i].vip)){
                                logger(LOG_INFO,"Failed to install vip %s", c->o.l[i].interface);
                            }
                            usleep(500);
                            if(install_netmask(c->o.l[i].interface, c->o.l[i].netmask)){
                                logger(LOG_INFO,"Failed to down interface %s", c->o.l[i].interface);
                            }else{
                                vip_status[i] = 1;
                                logger(LOG_INFO, "Slave is Dead, Check network");
                            }
                        }
                        if(send_http(c->o.direct_port, c->o.direct_ip, DUPLEXER_MYPLANE)){
                            logger(LOG_INFO,"Failed to Send HA status");
                        }  
                    }
                }
            }
        }
        alive_count = 0;
        sleep(1);
    }
}