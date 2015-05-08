/*
 * RpiBTSerialComm.h
 *
 *  Created on: 28 janv. 2015
 *      Author: brice
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#define FALSE					0x00000000
#define TRUE					0x00000001

#define DEVNAME					"/dev/rfcomm0"
#define SPSPEED					115200
#define SPPARITY				PARITY_NONE
#define EXIT_ABORT				-1
#define TIMEOUTWAITING4ANS_SEC	5000
#define TIMEOUTAFTERANS_SEC		20
#define PROTOCOL_VERSION		1

/* BlueTooth */
#define RX_BUFFER_SIZE	1024

/* CODE FUNCTION */
#define NB_COMMANDS		3

#define DISCOVER_WIFI	0
#define DISCONNECT		1
#define GET_NETWORK		2

/* WiFi */
#define IW_SCAN_HACK			0x8000

//#include "Network/wifiTools.h"
#include "stdint.h"
#include "wireless/iwlib.h"

extern int running;

#endif /* CONSTANTS_H_ */
