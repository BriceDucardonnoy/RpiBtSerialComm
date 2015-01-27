/*
 ============================================================================
 Name        : RpiBTSerialComm.c
 Author      : Brice DUCARDONNOY
 Version     :
 Copyright   : Copyright (c) 2015 Brice DUCARDONNOY
 Description : BlueTooth through serial device communication program in C
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ttys/ttys.h"

#define DEVNAME					"/dev/rfcomm0"
#define SPSPEED					115200
#define SPPARITY				PARITY_NONE
#define EXIT_ABORT				-1
#define TIMEOUTWAITING4ANS_SEC	5000
#define TIMEOUTAFTERANS_SEC		20

// TODO BDY: make an autogen.sh
static int initAndTalkWithBTDevice(void);

int main(void) {
	return initAndTalkWithBTDevice();
}

static int initAndTalkWithBTDevice(void) {
	Ttys ttys;
	int ret = 0;
	char buffer[128];
	*buffer=0;

	puts("Start application initialization"); /* prints Hello World */
	if((ttys = ttys_init()) == NULL) {
		fprintf(stderr, "Failed to create context\n");
		return EXIT_ABORT;
	}
	// BT 115200 8N1 on /dev/rfcomm0
	if((ret = ttys_setdevname(ttys, DEVNAME)) != TTYS_OK) {
		fprintf(stderr, "Failed to set device name to %s::%d returned\n", DEVNAME, ret);
		goto freeTty;
	}
	if((ret = ttys_setbauds(ttys, SPSPEED)) != TTYS_OK) {
		fprintf(stderr, "Failed to set speed to %d::%d returned\n", SPSPEED, ret);
		goto freeTty;
	}
	if((ret = ttys_setParity(ttys, SPPARITY)) != TTYS_OK) {
		fprintf(stderr, "Failed to parity to %d::%d returned\n", SPPARITY, ret);
		goto freeTty;
	}
	puts("Connect");
	if((ret = ttys_connect(ttys)) != TTYS_OK) {
		fprintf(stderr, "Failed to connect::%d returned\n", ret);
		goto freeTty;
	}
	puts("Connected. Wait for 2 seconds...");
	sleep(2);
	puts("Write something");
	ttys_sendstring(ttys, "Hello world from RPi!!!\r\n");
	puts("Wait for something");
	if((ret = ttys_empty_input_buffer(ttys)) != TTYS_OK) {
		fprintf(stderr, "Failed to empty buffer::%d returned\n", ret);
		goto freeTty;
	}
	if(ttys_getstring(ttys, buffer, 127, TIMEOUTWAITING4ANS_SEC, TIMEOUTAFTERANS_SEC) > 0) {
		printf("%s\n",buffer);
	}

	puts("Disconnect");
	if((ret = ttys_disconnect(ttys)) != TTYS_OK) {
		fprintf(stderr, "Failed to disconnect::%d returned\n", ret);
		goto freeTty;
	}

freeTty:
	printf("Free tty context\n");
	ttys_free(&ttys);

	return ret == TTYS_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}
