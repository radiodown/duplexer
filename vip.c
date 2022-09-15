#include "syshead.h"
#include "logger.h"

int install_vip(char* interface, char* ip){
	struct ifreq ifr;
	struct sockaddr_in* addr;
	int fd, ret;
	char iface[128] = {0,};

	/* vip interface : eg. ens33:1 */
	sprintf(iface,"%s:1",interface);

	/* Setting ifreq structure */
	memset(&ifr, 0, sizeof(struct ifreq));
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);

	addr = (struct sockaddr_in*)&ifr.ifr_addr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0){
		return 1;
	}

	/* install vip */
	inet_pton(AF_INET, ip, &addr->sin_addr);
    ret = ioctl(fd, SIOCSIFADDR, &ifr);
	if (ret < 0) {
		close(fd);
    	return 1;
  	}

	close(fd);
    logger(LOG_INFO, "Installed vip [%s] interface [%s]",ip ,interface );

	return 0;
}

int install_netmask(char* interface,char* netmask){
	struct ifreq ifr;
	struct sockaddr_in* addr;
	int fd, ret;
	char iface[128] = {0,};
	
	/* vip interface : eg. ens33:1 */
	sprintf(iface,"%s:1",interface);

	/* Setting ifreq structure */
	memset(&ifr, 0, sizeof(struct ifreq));
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);

	addr = (struct sockaddr_in*)&ifr.ifr_addr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0){
		return 1;
	}

	/* install netmask */
	inet_pton(AF_INET, netmask, &addr->sin_addr);
    ret = ioctl(fd, SIOCSIFNETMASK, &ifr);
	if (ret < 0) {
		close(fd);
    	return 1;
  	}

	close(fd);

    logger(LOG_INFO, "Installed netmask [%s] interface [%s]",netmask ,interface );

	return 0;
}


int check_vip(char* interface, char* ip){
	struct ifreq ifr;
	int fd, ret;
	char iface[128] = {0,};

	/* vip interface : eg. ens33:1 */
	sprintf(iface,"%s:1",interface);

	/* Setting ifreq structure */
	memset(&ifr, 0, sizeof ifr);
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0){
		return 1;
	}

	/* Get ip */
	ret = ioctl(fd, SIOCGIFADDR, &ifr);
	if (ret < 0) {
		close(fd);
    	return 1;
  	}

	/* inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr) */
	if(strcmp(ip,inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr))==0){
		close(fd);
        logger(LOG_DEBUG, "Checked Installed IP Successfully");
		return 0;
	}else{
        logger(LOG_DEBUG, "Failed Check Ip : Installed IP is [%s]",inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr));
		close(fd);
		return 1;
	}
}


int down_vip(char* interface){
	int fd, ret;
	struct ifreq ifr;
	char iface[128] = {0,};

	/* vip interface : eg. ens33:1 */
	sprintf(iface,"%s:1",interface);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0){
    	return 1; 
	}

	/* Setting ifreq structure */
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, iface, IFNAMSIZ-1);
	ifr.ifr_flags &= IFF_UP;

	/* Interface down */
	ret = ioctl(fd, SIOCSIFFLAGS, &ifr);
	// if (ret < 0) {
	// 	logger(LOG_INFO, "IOCTL");
	// 	close(fd);
    // 	return 1;
  	// }

	close(fd);
    logger(LOG_INFO, "Interface %s is down", interface);
	return 0;
}