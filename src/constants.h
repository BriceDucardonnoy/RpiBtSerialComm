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

/* Commands */
#define DISCOVER_WIFI			"Discover_WiFi"

/* WiFi */
#define IW_SCAN_HACK			0x8000

/* Global structure */
typedef struct {
	int sockFd;
	int clienttFd;
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

#endif /* CONSTANTS_H_ */
