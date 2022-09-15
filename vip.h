#ifndef VIP_H
#define VIP_H

int install_vip(char* interface, char* ip);
int install_netmask(char* interface,char* netmask);
int check_vip(char* interface, char* ip);
int down_vip(char* interface);

#endif