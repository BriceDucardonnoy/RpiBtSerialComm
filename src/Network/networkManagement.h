/*
 * ============================================================================
 * Name        : networkManagement.h
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

#ifndef NETWORK_NETWORKMANAGEMENT_H_
#define NETWORK_NETWORKMANAGEMENT_H_

typedef struct networkConf {
	int isDhcp;/* No need to put this flag on a byte because of alignment */
	int isWifi;
	char address[IFNAMSIZ];
	char netmask[IFNAMSIZ];
	char gateway[IFNAMSIZ];
	char primaryDns[IFNAMSIZ];
	char secondaryDns[IFNAMSIZ];
	char essid[IW_ESSID_MAX_SIZE];
} networkConf, *networkConf_t;

extern char *ifnames[];

extern int readNetworkInfo(stArgs_t args);
extern int readNetworkStatus(stArgs_t args);
/*!
 * \brief Start a thread listening network connectivity status
 *
 * Start a thread listening network connectivity status
 */
extern void startInterfaceMonitoring(glbCtx_t ctx);
/*!
 * \brief Stop the thread listening network connectivity status
 *
 * Stop the thread listening network connectivity status
 */
extern void terminateMonitoring(glbCtx_t ctx);

#endif /* NETWORK_NETWORKMANAGEMENT_H_ */
