/*
 * ============================================================================
 * Name        : wifiTools.h
 * Author      : Brice DUCARDONNOY
 * Created on  : 28 janvier 2015
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

/* Network specific */
#define NETWORK_LAN		0x00
#define NETWORK_WIFI	0x01

/* WiFi */
#define IW_SCAN_HACK			0x8000

//#include "Network/wifiTools.h"
#include "stdint.h"
#include "wireless/iwlib.h"

extern int running;

#endif /* CONSTANTS_H_ */
