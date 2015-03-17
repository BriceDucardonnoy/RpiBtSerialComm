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

#include "../constants.h"
#include "wifiTools.h"


wireless_scan_head * scanWifi(void) {
	printf("Enter in empty %s\n", __FUNCTION__);
	int 				skfd;			/* generic raw socket desc.	*/
	char				*dev = "wlan0";
	int					ret = EXIT_SUCCESS;
	wireless_scan_head 	wHead;
	wireless_scan 		*result;
	iwrange 			range;

	/* Create a channel to the NET kernel. */
	if((skfd = iw_sockets_open()) < 0) {
		perror("socket");
		return NULL;
	}

	/* Get some metadata to use for scanning */
	if (iw_get_range_info(skfd, dev, &range) < 0) {
		fprintf(stderr, "Error during iw_get_range_info. Aborting.\n");
		ret = EXIT_FAILURE;
		goto CleanAll;
	}

	/* Perform the scan */
	if (iw_scan(skfd, dev, range.we_version_compiled, &wHead) < 0) {
		fprintf(stderr, "Error during iw_scan. Aborting, reason: %d::%s\n", errno, strerror(errno));
		ret = EXIT_FAILURE;
	}

	/* Traverse the results */
	result = wHead.result;
	while (NULL != result) {
		printf("%s: level=%u, noise=%u, quality=%u\n",
			result->b.essid,
			result->stats.qual.level,
			result->stats.qual.noise,
			result->stats.qual.qual);
		result = result->next;
	}

CleanAll:
	printf("Clean all\n");
	iw_sockets_close(skfd);
	if(wHead.result != NULL) {
		result = wHead.result;
		wireless_scan *future;
		printf("Free ");
		do {
			printf("<%s> ", result->b.essid);
			future = result->next;
			free(result);
			result = future;
		} while(result != NULL);
		printf("\n");
	}

	return ret == EXIT_SUCCESS ? &wHead : NULL;
}
