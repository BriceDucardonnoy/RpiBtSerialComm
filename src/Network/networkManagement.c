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
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/inotify.h>
#include <pthread.h>
#include <fcntl.h>

#include "../RpiBTSerialComm.h"
#include "networkManagement.h"
#include "../constants.h"

static void printData(networkConf conf);
static int readWifi(networkConf_t conf, char *ifname);
static int readGateways(networkConf_t conf, char *devname);
static int readStaticDhcp(networkConf_t conf, char *ifname);
static int readDns(networkConf_t conf);

static void * listenNetworkConnectivity(void *userData);
static void printThreadJoinErrorText(int errCode);
/*
 * \brief Open the file related to <interface> and return its content
 * Open the file related to <interface>: /tmp/<interface>Status and read its content
 *
 * \return EXIT_ABORT if failed (-1) or the content of file:
 * -1	no network or update about the status available (script was launched with wrong arguments)
 * 0	www.google.com is reachable
 * 1	8.8.8.8 is reachable. Probably DNS server not configured or not reachable
 * 2	gateway or DNS reachable. Is the unit on a private network?
 */
static int updateConnectivityStatus(glbCtx_t ctx, char *interface);

static const int cnPathExtraLength = 12;// + 12 <=> '/tmp/' + 'Status' + '\0' string length
char *ifnames[] = {"wlan0", "eth0"};
// TODO BDY: write doc when updateStatus done

/*!
 * \brief Put in the output args the status of ETH0 and WLAN0 connectivity
 *
 * Put in the output args the status of ETH0 and WLAN0 connectivity.
 * This is get from the checkInterfaceStatus.sh bash script
 * Connectivity status per interface is:
 * -1	no network or update about the status available (script was launched with wrong arguments)
 * 0	www.google.com is reachable
 * 1	8.8.8.8 is reachable. Probably DNS server not configured or not reachable
 * 2	gateway or DNS reachable. Is the unit on a private network?
 *
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int readNetworkStatus(stArgs_t args) {
	// Map in SHM?
	// TODO BDY: NYI readNetworkStatus
	return EXIT_SUCCESS;
}

int readNetworkInfo(stArgs_t args)
{
	int fd;
	int ret = EXIT_SUCCESS;
	struct ifreq ifr;
	networkConf_t conf = NULL;
	char *ifname;
	printf("Enter in %s\n", __FUNCTION__);
	conf = calloc(1, sizeof(networkConf));
	// Extract the configuration asked: NETWORK_LAN or NETWORK_WIFI
	switch(PROTOCOL_VERSION) {
	case 1:
		conf->isWifi = args->input[4];
		break;
	default:
		fprintf(stderr, "Unvalid protocol\n");
		return EXIT_FAILURE;
	};

	ifname = conf->isWifi ? ifnames[0] : ifnames[1];

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		fprintf(stderr, "Cannot get control socket: %d::%s\n", errno, strerror(errno));
		return EXIT_FAILURE;
	}
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);

	// Address
	int rc = ioctl(fd, SIOCGIFADDR, &ifr);
	if(rc < 0) {
		fprintf(stderr, "Failed to get IP address for %s: %d::%s. Try with %s.\n", ifname, errno, strerror(errno), ifnames[1]);
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
    readGateways(conf, ifname);

    // DHCP / Static
    readStaticDhcp(conf, ifname);// TODO BDY: manual for wifi, neither static nor dhcp

    // DNS
    readDns(conf);

    // WiFi
    if(conf->isWifi) {
    	readWifi(conf, ifname);
    }

	printData(*conf);

	// Copy result into output
	if((args->output = malloc(sizeof(networkConf))) == NULL) {
		fprintf(stderr, "Failed to allocate memory in %s: %d::%s\n", __FUNCTION__, errno, strerror(errno));
		ret = EXIT_FAILURE;
		goto CleanAll;
	}
	memcpy(args->output, conf, sizeof(networkConf));
	args->outputLength = sizeof(networkConf);

CleanAll:
	printf("%s: Clean all\n", __FUNCTION__);
	close(fd);
//	if(conf->address) free(conf->address);
//	if(conf->netmask) free(conf->netmask);
//	if(conf->gateway) free(conf->gateway);
	if(conf) free(conf);

    return ret;
}

static int readWifi(networkConf_t conf, char *ifname) {
	wireless_config config;
	int skfd;
	if((skfd = iw_sockets_open()) < 0) {
		perror("Socket");
		return EXIT_FAILURE;
	}
	int rc = iw_get_basic_config(skfd, ifnames[0], &config);
	printf("WIFI ESSID: (%d) %s::%d\n", rc, config.essid, config.has_essid);
	iw_sockets_close(skfd);
	if(config.has_essid) {
		strncpy(conf->essid, config.essid, IW_ESSID_MAX_SIZE - 1);
		return 0;
	}
	return -1;
}

static int readGateways(networkConf_t conf, char *devname) {
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
		return EXIT_FAILURE;
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
	return EXIT_SUCCESS;
}

static int readStaticDhcp(networkConf_t conf, char *ifname) {
	FILE *fp;
	char buffer[16];
	char *cmd = malloc(96);

	if(sprintf(cmd, "cat /etc/network/interfaces | grep ^\"iface\\ %s \" | awk -F ' ' '{print $4}'", ifname) == 0) {
		fprintf(stderr, "Failed to create request to know weather network is DHCP or static\n");
		return EXIT_FAILURE;
	}
	printf("Request is <%s>\n", cmd);

	fp = popen(cmd, "r");
	free(cmd);
	if(fgets(buffer, sizeof(buffer), fp) != NULL) {
		printf("%s", buffer);
		if(strncmp(buffer, "static", 6) == 0) {
			conf->isDhcp = FALSE;
		}
		else if(strncmp(buffer, "dhcp", 4) == 0) {
			conf->isDhcp = TRUE;
		}
		else {
			fprintf(stderr, "Error, %s is neither static nor dhcp\n", ifname);
			pclose(fp);
			return EXIT_FAILURE;
		}

	}
	pclose(fp);
	return EXIT_SUCCESS;
}

static int readDns(networkConf_t conf) {
	FILE *dnsFd;
	char dns[128];
	char *p;
	int dnsIdx 				= 0;
	int ifaceSz 			= conf->isWifi ? strlen("iface wlanX inet") : strlen("iface ethX inet");
	char *ethToFind 		= calloc(1, ifaceSz + 1);
	bool bEthFound 			= FALSE;// Set to true if /etc/resolv.conf is used
	const char *dnsKeyWord	= "dns-nameservers\0";
	const char *dnsFileInfo = "/etc/network/interfaces";
//	const char *dnsFileInfo = "/etc/resolv.conf";// Set bEthFound to true if /etc/resolv.conf is used
//	const char *dnsKeyWord	= "nameserver\0";
	if(ethToFind == NULL) {
		perror("Can't allocate memory to read DNS");
		return EXIT_FAILURE;
	}
	snprintf(ethToFind, ifaceSz, "iface %s inet ", conf->isWifi ? ifnames[0] : ifnames[1]);
	printf("%s:: Look for <%s>...<%s> into %s\n", __FUNCTION__, dnsKeyWord, ethToFind, dnsFileInfo);

	dnsFd = fopen(dnsFileInfo, "r");
	if (!dnsFd) {
		fprintf(stderr, "Fail to open %s: %d::%s\n", dnsFileInfo, errno, strerror(errno));
		free(ethToFind);
		return EXIT_FAILURE;
	}

	while (fgets(dns, 128, dnsFd)) {
		if(dnsIdx >= 2) break;
//		printf("%s:: Line is <%s>\n", __FUNCTION__, dns);
		p = dns;
		// Shift the pointer until we reach a non-space character
		while(isspace(*p) && *p != '\0') {
			p++;
		}
		/*
		 * Part necessary if we check into /etc/network/interfaces
		 */
		if(strncmp(p, ethToFind, strlen(ethToFind)) == 0) {
//			printf("%s:: FOUND\n", __FUNCTION__);
			bEthFound = TRUE;
		}
		else if(bEthFound == TRUE && strncmp(p, "iface ", 6) == 0) {
			// We already have parsed all the relevant part. Now we can leave the loop
//			printf("%s:: We are done now\n", __FUNCTION__);
			break;
		}
		/*
		 * End of specific part
		 */
		if (bEthFound == TRUE && strncmp(p, dnsKeyWord, strlen(dnsKeyWord)) == 0) {
//			printf("read <%s>\n", dns);
//			p = &dns[11];
			p += strlen(dnsKeyWord) + 1;
			while (*p == ' ' || *p == '\t') {// <=> to isspace()
				p++;
			}
			if(*p) {
				char *line = strsep(&p, "\n");
				char *dns1, *dns2;
				dns1 = strtok(line, " ");
				dns2 = strtok(NULL, " ");
				printf("1st elem <%s>\n", dns1);
				printf("2nd elem <%s>\n", dns2);
				if(dns1 != NULL) {
					switch(dnsIdx) {
					case 0:
						strncpy(conf->primaryDns, dns1, IFNAMSIZ - 1);
						dnsIdx++;
						break;
					case 1:
						strncpy(conf->secondaryDns, dns1, IFNAMSIZ - 1);
						dnsIdx++;
						break;
					default:
						break;
					};
				}// DNS1 != NULL
				if(dns2 != NULL) {
					switch(dnsIdx) {
					case 0:
						strncpy(conf->primaryDns, dns2, IFNAMSIZ - 1);
						dnsIdx++;
						break;
					case 1:
						strncpy(conf->secondaryDns, dns2, IFNAMSIZ - 1);
						dnsIdx++;
						break;
					default:
						break;
					};
				}// DNS2 != NULL
			}
		}
	}

	fclose(dnsFd);
	free(ethToFind);
	return EXIT_SUCCESS;
}

static void printData(networkConf conf) {
	printf("Network data: \n"
			"\tIP: %s\n"
			"\tNetmask: %s\n"
			"\tGateway: %s\n"
			"\tDNS1: %s\n"
			"\tDNS2: %s\n"
			"\tWiFi/LAN: %s\n"
			"\tESSID: %s\n"
			"\tStatic/DHCP: %s\n",
			conf.address, conf.netmask, conf.gateway, conf.primaryDns, conf.secondaryDns,
			conf.isWifi == FALSE ? "LAN" : "WiFi",
			conf.isWifi == TRUE ? conf.essid : "<>",
			conf.isDhcp == FALSE ? "Static" : "DHCP");
}

/*!
 * \brief Start a thread listening network connectivity status
 *
 * Start a thread listening network connectivity status
 */
void startInterfaceMonitoring(glbCtx_t ctx) {
	ctx->monitorInterface = TRUE;
	pthread_mutex_init(&ctx->monitorInterfaceMutex, NULL);
	pthread_create(&ctx->monitorInterfaceThread, NULL, listenNetworkConnectivity, (void *) ctx);
}

/*!
 * \brief Stop the thread listening network connectivity status
 *
 * Stop the thread listening network connectivity status
 */
void terminateMonitoring(glbCtx_t ctx) {
	pthread_mutex_lock(&ctx->monitorInterfaceMutex);
	ctx->monitorInterface = FALSE;
	pthread_mutex_unlock(&ctx->monitorInterfaceMutex);
	printThreadJoinErrorText(pthread_join(ctx->monitorInterfaceThread, NULL));
	pthread_mutex_destroy(&ctx->monitorInterfaceMutex);
}

static void printThreadJoinErrorText(int errCode) {
	switch(errCode) {
	case 0: printf("Thread successfully joined.\n");
		break;
	case EDEADLK: fprintf(stderr, "A deadlock was detected or thread specifies the calling thread.\n");
		break;
	case EINVAL: fprintf(stderr, "Thread is not a joinable thread or another thread is already waiting to join with this thread.\n");
		break;
	case ESRCH: fprintf(stderr, "No thread with the ID thread could be found.\n");
		break;
	default : printf("Got join code %d.\n", errCode);
		break;
	}
}

/*!
 * \brief Thread listening for network status update
 * Thread listening for network status update with iNotify system.
 * Files updated are <interface name>Status located in /tmp.
 * Eg. eth0Status and wlan0Status
 *
 * \return EXIT_SUCCESS or EXIT_ABORT if inotify system failed to initialize
 */
static void * listenNetworkConnectivity(void *userData) {
	int idx;
	int inotifyFd;
	int wds[NB_INTERFACE_MONITORED];
	int value;
	char *interfacePath;
	glbCtx_t ctx = (glbCtx_t) userData;
	size_t bufferLength = sizeof(struct inotify_event);// Biggest interfaces name + status won't be bigger than 11 bytes
	ssize_t readNb;
	char buffer[bufferLength];
	struct inotify_event *event;

	// Init
	inotifyFd = inotify_init();
	if(inotifyFd == -1) {
		fprintf(stderr, "Failed to init inotify: %d::%s\n", errno, strerror(errno));
		pthread_exit(NULL);
		return NULL;
	}
	// Add watch
	for(idx = 0 ; idx < NB_INTERFACE_MONITORED ; idx++) {
		interfacePath = calloc(1, strlen(ifnames[idx]) + cnPathExtraLength);
		if(interfacePath == NULL) {
			perror("Can't allocate memory");
			continue;
		}
		snprintf(interfacePath, strlen(ifnames[idx]) + cnPathExtraLength, "/tmp/%sStatus", ifnames[idx]);
		wds[idx] = inotify_add_watch(inotifyFd, interfacePath, IN_CLOSE_WRITE);
		if(wds[idx] == -1) {
			fprintf(stderr, "Failed to add inotify watch to %s: %d::%s\n", interfacePath, errno, strerror(errno));
		}
		free(interfacePath);
	}
	// Listen
	while(ctx->monitorInterface) {
		readNb = read(inotifyFd, buffer, bufferLength);
		if(readNb < 0) {
			perror("Read");
		}
		event = (struct inotify_event*) buffer;
		for(idx = 0 ; idx < NB_INTERFACE_MONITORED ; idx++) {
			if(event->wd == wds[idx]) {
				printf("%s event received\n", ifnames[idx]);
				value = updateConnectivityStatus(ctx, ifnames[idx]);
				// Update context network status
				pthread_mutex_lock(&ctx->monitorInterfaceMutex);
				ctx->interfaceStatus[idx] = value;
				pthread_mutex_unlock(&ctx->monitorInterfaceMutex);
				break;
			}
		}
	}
	// Monitoring ended -> remove listening
	for(idx = 0 ; idx < NB_INTERFACE_MONITORED ; idx++) {
		if(inotify_rm_watch(inotifyFd, wds[idx]) == -1) {
			perror("Remove watch failed: ");
		}
	}

	pthread_exit(NULL);
	return NULL;
}

/*
 * \brief Open the file related to <interface> and return its content
 * Open the file related to <interface>: /tmp/<interface>Status and read its content
 *
 * \return EXIT_ABORT if failed (-1) or the content of file:
 * -1	no network or update about the status available (script was launched with wrong arguments)
 * 0	www.google.com is reachable
 * 1	8.8.8.8 is reachable. Probably DNS server not configured or not reachable
 * 2	gateway or DNS reachable. Is the unit on a private network?
 */
static int updateConnectivityStatus(glbCtx_t ctx, char *interface) {
	int fd;
	int rc = EXIT_SUCCESS;
	char *interfacePath = NULL;
	const int bufferSz = 3;// '-' '1' '\0'
	char *buffer = calloc(1, bufferSz);

	interfacePath = calloc(1, strlen(interface) + cnPathExtraLength);
	if(interfacePath == NULL) {
		perror("Can't allocate memory");
		return EXIT_ABORT;
	}
	snprintf(interfacePath, strlen(interface) + cnPathExtraLength, "/tmp/%sStatus", interface);

	fd = open(interfacePath, O_RDONLY);
	if(fd == -1) {
		fprintf(stderr, "Failed to open %s: %d::%s\n", interfacePath, errno, strerror(errno));
		rc = EXIT_ABORT;
		goto CleanAll;
	}
	if(read(fd, buffer, bufferSz) == -1) {
		fprintf(stderr, "Failed to read %s: %d::%s\n", interfacePath, errno, strerror(errno));
		rc = EXIT_ABORT;
		goto CleanAll;
	}
	printf("Read %s\t", buffer);
	if(close(fd) == -1) {
		fprintf(stderr, "Failed to close %s: %d::%s\n", interfacePath, errno, strerror(errno));
	}

	// Convert from char * to integer
	rc = atoi(buffer);
	printf("Converted int value is %d\n", rc);
CleanAll:
	if(buffer) free(buffer);
	if(interfacePath) free(interfacePath);
	return rc;
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
