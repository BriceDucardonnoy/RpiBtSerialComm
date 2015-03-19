/*
 ============================================================================
 Name        : RpiBTSerialComm.c
 Author      : Brice DUCARDONNOY
 Version     :
 Copyright   : Copyright (c) 2015 Brice DUCARDONNOY
 Description : BlueTooth through serial device communication program in C
 ============================================================================
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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "constants.h"
#include "RpiBTSerialComm.h"
#include "bluetoothUtils.h"
#include "Network/wifiTools.h"
#include "communicationProtocol.h"

static int (*commMethods[NB_COMMANDS]) (stArgs_t args) = { scanWifi };

int main(int argc, char **argv) {
	GlbCtx_t ctx = initContext();
//	simpleScan();// For the fun
//	rfcommServer();
//	return initAndTalkWithBTDevice();
	if(argc == 2) {
		if(strstr(argv[1], "ScanWifi") != NULL) {
			printf("Test wifi scan\n");
//			(*ctx->commMethods[0])(NULL);// Normal call
//			if((ctx->wHead = scanWifi()) == NULL) {// Classic call
//			if((ctx->wHead = (wireless_scan_head*) (*ctx->commMethods[0])(NULL)) == NULL) {
			stArgs_t args = malloc(sizeof(struct stArgs));
			if(commMethods[0](args) == 0) {
				return EXIT_FAILURE;
			}
			else {
				cleanWirelessScanHeadContent(ctx->wHead);
				return EXIT_SUCCESS;
			}
		}
		else if(strstr(argv[1], "TestProtocol") != NULL) {
			printf("Test protocol\n");
			test();
		}
		return EXIT_SUCCESS;
	}

	if(wait4connect(ctx, 20) != 0) {
		fprintf(stderr, "Failed during wait4connect\n");
		return EXIT_FAILURE;
	}
	readAndRepeat(ctx);
	// Close all FD
	if(ctx->clienttFd >= 0) {
		puts("Close accept FD");
		close(ctx->clienttFd);
	}
	if(ctx->sockFd >= 0) {
		puts("Close socket FD");
		close(ctx->sockFd);
	}
	destroyContext(ctx);
	return EXIT_SUCCESS;
}

int callFunction(int funcCode, stArgs_t args) {
	if(funcCode >= 0 && funcCode < NB_COMMANDS) {
		return commMethods[funcCode](args);
	}
	return -1;
}

GlbCtx_t initContext(void) {
	GlbCtx_t ctx = malloc(sizeof(GlbCtx));
//	ctx->wHead = malloc(sizeof(wireless_scan_head));

	/* Init commands for communication protocol */
//	commMethods[0] = (void *) *scanWifi;
	return ctx;
}

void destroyContext(GlbCtx_t ctx) {
	if(!ctx) return ;
	if(ctx->wHead) {
		cleanWirelessScanHeadContent(ctx->wHead);
		free(ctx->wHead);
	}
	free(ctx);
}

// TODO BDY: monitor signals for scan anc pairing

