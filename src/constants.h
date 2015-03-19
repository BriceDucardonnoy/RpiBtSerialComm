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

/* Communication */
#define WIDTH  (8 * sizeof(crc_t))
#define TOPBIT (1 << (WIDTH - 1))
#define POLYNOMIAL 0x8005

/* CODE FUNCTION */
#define NB_COMMANDS		1

#define DISCOVER_WIFI	0

/* WiFi */
#define IW_SCAN_HACK			0x8000

//#include "Network/wifiTools.h"
#include "stdint.h"
#include "wireless/iwlib.h"

/*
 * The width of the CRC calculation and result.
 * Modify the typedef for a 16 or 32-bit CRC standard.
 */
typedef uint8_t crc_t;

#endif /* CONSTANTS_H_ */
