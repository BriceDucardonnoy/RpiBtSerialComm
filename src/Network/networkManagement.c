/*
 * ============================================================================
 * Name        : networkManagement.c
 * Author      : Brice DUCARDONNOY
 * Created on  : 8 mai 2015
 * Version     :
 * Copyright   : Copyright © 2015 Brice DUCARDONNOY
 * Description : Program in C
 * ============================================================================
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * - The above copyright notice and this permission notice shall be included 
 * 	in all copies or substantial portions of the Software.
 * - The Software is provided "as is", without warranty of any kind, express 
 * 	or implied, including but not limited to the warranties of merchantability, 
 * 	fitness for a particular purpose and noninfringement.
 * 
 * In no event shall the authors or copyright holders be liable for any claim, 
 * damages or other liability, whether in an action of contract, tort or otherwise, 
 * arising from, out of or in connection with the software or the use or other 
 * dealings in the Software.
 */

#include <net/if.h>
#include <net/route.h>
#include <linux/sockios.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

//#include <netinet/in.h>
//#include <arpa/nameser.h>
//#include <resolv.h>

#include "../RpiBTSerialComm.h"
#include "networkManagement.h"
#include "../constants.h"

//extern struct res_state state;

static void printData(networkConf conf);
static void readGateways(networkConf_t conf, char *devname);

int readIPAddresses(stArgs_t args)
{
	int fd;
	int ret = EXIT_SUCCESS;
	struct ifreq ifr;
	networkConf_t conf = NULL;
	char *ethName = "eth0";
	char *wifiName = "wlan0";


	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		fprintf(stderr, "Cannot get control socket: %d::%s\n", errno, strerror(errno));
		return EXIT_FAILURE;
	}
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, ethName, IFNAMSIZ-1);

	conf = calloc(1, sizeof(networkConf));

	// Address
	if(ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
		fprintf(stderr, "Failed to get IP address: %d::%s\n", errno, strerror(errno));
		ret = EXIT_FAILURE;
		goto CleanAll;
	}
	strncpy(conf->address, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), IFNAMSIZ - 1);

    // Netmask
    if(ioctl(fd, SIOCGIFNETMASK, &ifr) < 0) {
    	fprintf(stderr, "Failed to get netmask: %d::%s\n", errno, strerror(errno));
    	ret = EXIT_FAILURE;
    	goto CleanAll;
    }
    strncpy(conf->netmask, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), IFNAMSIZ - 1);

    // Gateway
    readGateways(conf, ethName);
    // DNS

	printData(*conf);
//	memcpy(args->output, conf, sizeof(networkConf));

CleanAll:
	printf("%s: Clean all\n", __FUNCTION__);
	close(fd);
//	if(conf->address) free(conf->address);
//	if(conf->netmask) free(conf->netmask);
//	if(conf->gateway) free(conf->gateway);
	if(conf) free(conf);

    return ret;
}

static void readGateways(networkConf_t conf, char *devname) {
	FILE *routeFd;
	char route[256];
	char name[64];
	in_addr_t dest, mask;
	struct in_addr gway;
	int flags, refcnt, use, metric, mtu, win, irtt;
	int nread;

	routeFd = fopen("/proc/net/route", "r");
	if (!routeFd) {
		fprintf(stderr, "Fail to open /proc/net/route: %d::%s\n", errno, strerror(errno));
		return;
	}

	while(fgets(route, 255, routeFd)) {
		nread = fscanf(routeFd, "%63s%X%X%X%d%d%d%X%d%d%d\n",
				name, &dest, &gway.s_addr, &flags, &refcnt, &use, &metric, &mask, &mtu, &win, &irtt);
		if (nread != 11) {
			break;
		}
		// Looking for default gateway for the given ifname
		if ((flags & (RTF_UP|RTF_GATEWAY)) == (RTF_UP|RTF_GATEWAY)
				&& dest == 0
				&& strcmp(devname, name) == 0) {
			printf("Default gateway found: %08X, %s\n", gway.s_addr, inet_ntoa(gway));
			strncpy(conf->gateway, inet_ntoa(gway), IFNAMSIZ - 1);
			break;
		}
	}
	fclose(routeFd);
}

static void printData(networkConf conf) {
	printf("Network data: \n"
			"\tIP: %s\n"
			"\tNetmask: %s\n"
			"\tGateway: %s\n"
			"\tDNS1: %s\n"
			"\tDNS2: %s\n",
			conf.address, conf.netmask, conf.gateway, conf.primaryDns, conf.secondaryDns);
}

/*
 * \brief Secure a string injected into a System command by removing special characters allowing malicious code
 * \ingroup Security
 */
//void secureString(String_t Unsafe) {
//	if (Unsafe == NULL)
//		return;
//	tb_StrRepl(Unsafe, "%", "");
//	tb_StrRepl(Unsafe, " ", "");
//	tb_StrRepl(Unsafe, "&", "");
//	tb_StrRepl(Unsafe, "|", "");
//	tb_StrRepl(Unsafe, ";", "");
//	tb_StrRepl(Unsafe, "-", "");
//	tb_StrRepl(Unsafe, "$", "");
//	tb_StrRepl(Unsafe, "`", "");
//	tb_StrRepl(Unsafe, "'", "");
//	tb_StrRepl(Unsafe, "\"", "");
//	tb_StrRepl(Unsafe, "/", "");
//}
