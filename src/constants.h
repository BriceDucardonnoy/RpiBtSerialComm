/*
 * RpiBTSerialComm.h
 *
 *  Created on: 28 janv. 2015
 *      Author: brice
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#define DEVNAME					"/dev/rfcomm0"
#define SPSPEED					115200
#define SPPARITY				PARITY_NONE
#define EXIT_ABORT				-1
#define TIMEOUTWAITING4ANS_SEC	5000
#define TIMEOUTAFTERANS_SEC		20

#define DISCOVER_WIFI			"Discover_WiFi"

typedef struct {
	int sockFd;
	int clienttFd;
} GlbCtx;
typedef GlbCtx * GlbCtx_t;

#endif /* CONSTANTS_H_ */
