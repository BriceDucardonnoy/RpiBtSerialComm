/*
 * ============================================================================
 * Name        : wifiTools.h
 * Author      : Brice DUCARDONNOY
 * Created on  : 17 mars 2015
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

#ifndef NETWORK_WIFITOOLS_H_
#define NETWORK_WIFITOOLS_H_

#include "../constants.h"
#include "../RpiBTSerialComm.h"
#include "wireless/iwlib.h"

/*
 * Scan state and meta-information, used to decode events...
 */
typedef struct iwscan_state
{
  /* State */
  int			ap_num;		/* Access Point number 1->N */
  int			val_index;	/* Value in table 0->(N-1) */
} iwscan_state;

/*!
 * \brief Look for the visible WiFi network
 *
 * Look for the visible WiFi network and return a list of network.
 * It's up to the caller to free the list of network with clean_wireless_scan_head_content method.
 *
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
extern int scanWifi(stArgs_t args);
/*!
 * \brief Clean a wireless_scan_head content.
 *
 * 	Clean a wireless_scan_head content.
 * 	It's up to the caller to clean then the wireless_scan_head itself (free(wireless_scan_head)).
 */
extern void cleanWirelessScanHeadContent(wireless_scan_head *wHead);

#endif /* NETWORK_WIFITOOLS_H_ */
