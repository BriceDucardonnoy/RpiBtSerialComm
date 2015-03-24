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
#include <signal.h>

#include "constants.h"
#include "RpiBTSerialComm.h"
#include "bluetoothUtils.h"
#include "Network/wifiTools.h"
#include "communicationProtocol.h"

#define FUNC(X) {.commMethods = X}

static int testRpi(glbCtx_t ctx, int argc, char **argv);

//static int (*commMethods[]) (stArgs_t args) = { scanWifi };
static stCommFunc commFuncs[] = {
//	{.commMethods = scanWifi},
	FUNC(scanWifi),
	{ NULL }
};
int running;

void sigterm_handler(int sig) {
	// SIGTERM is sent by process SandBoxMng to notify the sandbox to gracefully exit
	printf("sigterm caught !\n");
	running = FALSE;
}


void sigsegv_handler(int sig) {
	fprintf(stderr, "sigsegv caught !\n");
	// FIXME BDY: __dump_call_stack
//	__dump_call_stack();
	exit(EXIT_FAILURE);
}


void register_signal_handlers(void) {
	struct sigaction TERM_action;
	TERM_action.sa_handler  = sigterm_handler;
	sigemptyset(&TERM_action.sa_mask);
	TERM_action.sa_flags    = SA_RESTART;
	sigaction(SIGTERM,  &TERM_action, NULL);

	struct sigaction SEGV_action;
	SEGV_action.sa_handler  = sigsegv_handler;
	sigemptyset(&SEGV_action.sa_mask);
	SEGV_action.sa_flags    = SA_RESTART;
	sigaction(SIGSEGV,  &SEGV_action, NULL);
}

int main(int argc, char **argv) {
	glbCtx_t ctx = initContext();
	/* Old tests */
//	simpleScan();// For the fun
//	rfcommServer();
//	return initAndTalkWithBTDevice();

	// Tests
	if(argc == 2) {
		return testRpi(ctx, argc, argv);
	}
	// End of tests

	if(wait4connect(ctx, 20) != 0) {
		fprintf(stderr, "Failed during wait4connect\n");
		return EXIT_FAILURE;
	}
	running = TRUE;
	// Infinite loop
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

static int testRpi(glbCtx_t ctx, int argc, char **argv) {
	if(strstr(argv[1], "ScanWifi") != NULL) {
		printf("Test wifi scan\n");
//		if((ctx->wHead = (wireless_scan_head*) (*ctx->commMethods[0])(NULL)) == NULL) {
		stArgs_t args = malloc(sizeof(struct stArgs));
		args->ctx = ctx;
//		int ret = commMethods[0](args);
		int ret = commFuncs[0].commMethods(args);
		args->ctx = NULL;
		free(args);
		destroyContext(ctx);
		return ret;
	}
	else if(strstr(argv[1], "TestProtocol") != NULL) {
		printf("Test protocol\n");
		testProtocol(ctx);
	}
	return EXIT_SUCCESS;
}

int callFunction(int funcCode, stArgs_t args) {
	if(funcCode >= 0 && funcCode < NB_COMMANDS) {
		return commFuncs[funcCode].commMethods(args);
	}
	return EXIT_ABORT;
}

glbCtx_t initContext(void) {
	glbCtx_t ctx = calloc(1, sizeof(glbCtx));
//	ctx->wHead = malloc(sizeof(wireless_scan_head));

	/* Init commands for communication protocol */
//	commMethods[0] = (void *) *scanWifi;
	return ctx;
}

void destroyContext(glbCtx_t ctx) {
	if(!ctx) return ;
	if(ctx->wHead) {
		cleanWirelessScanHeadContent(ctx->wHead);
		free(ctx->wHead);
	}
	free(ctx);
}

void cleanArgs(stArgs_t args) {
	printf("Enter in function %s\n", __FUNCTION__);
	if(!args) return;
	if(args->input) {
		free(args->input);
		args->input = NULL;
	}
	if(args->output) {
		free(args->output);
		args->output = NULL;
	}
}

// TODO BDY: monitor signals for scan pairing
// TODO BDY: test with Valgrind

