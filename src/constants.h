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
#define DISCOVER_WIFI	0

/* WiFi */
#define IW_SCAN_HACK			0x8000

#include "Network/wifiTools.h"

/*
 * The width of the CRC calculation and result.
 * Modify the typedef for a 16 or 32-bit CRC standard.
 */
typedef uint8_t crc_t;
typedef void * func_pointer(int);

/* Global structure */
typedef struct {
	/* BlueTooth communication */
	int sockFd;/* FD for BlueTooth serial communication */
	int clienttFd;/* Another FD... */
	/* Communication protocol */
	crc_t crcTable[256];/* CRC calculation table */
	/* Function pointer array */
	void *commMethods;
	/* WiFi */
	wireless_scan_head *wHead;/* Scan array response */
} GlbCtx;
typedef GlbCtx * GlbCtx_t;

/*
 * Scan state and meta-information, used to decode events...
 */
typedef struct iwscan_state
{
  /* State */
  int			ap_num;		/* Access Point number 1->N */
  int			val_index;	/* Value in table 0->(N-1) */
} iwscan_state;

extern GlbCtx_t initContext(void);
extern void destroyContext(GlbCtx_t ctx);

#endif /* CONSTANTS_H_ */
