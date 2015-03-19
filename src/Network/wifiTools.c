/*
 * ============================================================================
 * Name        : wifiTools.c
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

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "wifiTools.h"
#include "../constants.h"

/*!
 * \brief Look for the visible WiFi network
 *
 * Look for the visible WiFi network and return a list of network.
 * It's up to the caller to free the list of network with clean_wireless_scan_head_content method.
 */
//wireless_scan_head * scanWifi(stArgs_t args) {
int scanWifi(stArgs_t args) {
	printf("Enter in %s\n", __FUNCTION__);
	int 				skfd;			/* generic raw socket desc.	*/
	char				*dev = "wlan0";
	int					ret = EXIT_SUCCESS;
	wireless_scan_head 	*wHead = malloc(sizeof(wireless_scan_head));
	wireless_scan 		*result;
	iwrange 			range;

	/* Create a channel to the NET kernel. */
	if((skfd = iw_sockets_open()) < 0) {
		perror("Socket");
		return EXIT_FAILURE;
	}

	/* Get some metadata to use for scanning */
	if (iw_get_range_info(skfd, dev, &range) < 0) {
		fprintf(stderr, "Error during iw_get_range_info. Aborting.\n");
		ret = EXIT_FAILURE;
		goto CleanAll;
	}

	/* Perform the scan */
	if (iw_scan(skfd, dev, range.we_version_compiled, wHead) < 0) {
		fprintf(stderr, "Error during iw_scan. Aborting, reason: %d::%s\n", errno, strerror(errno));
		ret = EXIT_FAILURE;
		goto CleanAll;
	}


	/* Traverse the results */
	result = wHead->result;
	while (NULL != result) {
		printf("%s: level=%u, noise=%u, quality=%u, crypted=0x%08X, Encryption key=%s\n",
			result->b.essid,
			result->stats.qual.level,
			result->stats.qual.noise,
			result->stats.qual.qual,
			result->b.key_flags,
			(result->b.key_flags & IW_ENCODE_DISABLED) == 0 ? "on" : "off");
		result = result->next;
	}

CleanAll:
//	printf("Clean all\n");
	iw_sockets_close(skfd);
	if(ret != EXIT_SUCCESS) {
		printf("Failure append, so clean wireless scan result\n");
		cleanWirelessScanHeadContent(wHead);
	}

//	return ret == EXIT_SUCCESS ? wHead : NULL;
	return ret;
}

/*!
 * \brief Clean a wireless_scan_head content.
 *
 * 	Clean a wireless_scan_head content.
 * 	It's up to the caller to clean then the wireless_scan_head itself (free(wireless_scan_head)).
 */
void cleanWirelessScanHeadContent(wireless_scan_head *wHead) {
	printf("Enter in %s\n", __FUNCTION__);
	wireless_scan *current, *future;

	if(wHead->result != NULL) {
		current = wHead->result;
		printf("Free ");
		do {
			printf("<%s> ", current->b.essid);
			future = current->next;
			free(current);
			current = future;
		} while(future != NULL);
		printf("\n");
	}
}
