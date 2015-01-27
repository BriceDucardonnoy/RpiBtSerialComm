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
#include <stdint.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>

#include "ttys/ttys.h"

#define DEVNAME					"/dev/rfcomm0"
#define SPSPEED					115200
#define SPPARITY				PARITY_NONE
#define EXIT_ABORT				-1
#define TIMEOUTWAITING4ANS_SEC	5000
#define TIMEOUTAFTERANS_SEC		20

static int simpleScan(void);
static int rfcommServer(void);
static int initAndTalkWithBTDevice(void);

int main(void) {
	simpleScan();
//	rfcommServer();
	return initAndTalkWithBTDevice();
}

static int simpleScan(void) {
	inquiry_info *ii = NULL;
	int max_rsp, num_rsp;
	int dev_id, sock, len, flags;
	int i;
	char addr[19] = { 0 };
	char name[248] = { 0 };

	dev_id = hci_get_route(NULL);
	sock = hci_open_dev( dev_id );
	if (dev_id < 0 || sock < 0) {
		perror("opening socket");
		exit(1);
	}

	len  = 8;
	max_rsp = 255;
	flags = IREQ_CACHE_FLUSH;
	ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));

	num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
	if( num_rsp < 0 ) perror("hci_inquiry");

	for (i = 0; i < num_rsp; i++) {
		ba2str(&(ii+i)->bdaddr, addr);
		memset(name, 0, sizeof(name));
		if (hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name),
				name, 0) < 0)
			strcpy(name, "[unknown]");
		printf("%s  %s\n", addr, name);
	}

	free( ii );
	close( sock );
	return 0;
}

static int rfcommServer(void) {
	struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
	char buf[1024] = { 0 };
	int s, client, bytes_read;
	socklen_t opt = sizeof(rem_addr);

	// allocate socket
	s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

	// bind socket to port 1 of the first available
	// local bluetooth adapter
	loc_addr.rc_family = AF_BLUETOOTH;
	loc_addr.rc_bdaddr = *BDADDR_ANY;
	loc_addr.rc_channel = (uint8_t) 1;
	bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

	// put socket into listening mode
	listen(s, 1);

	// accept one connection
	client = accept(s, (struct sockaddr *)&rem_addr, &opt);

	ba2str( &rem_addr.rc_bdaddr, buf );
	fprintf(stderr, "accepted connection from %s\n", buf);
	memset(buf, 0, sizeof(buf));

	// read data from the client
	bytes_read = read(client, buf, sizeof(buf));
	if( bytes_read > 0 ) {
		printf("received [%s]\n", buf);
	}

	// close connection
	close(client);
	close(s);
	return 0;
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
