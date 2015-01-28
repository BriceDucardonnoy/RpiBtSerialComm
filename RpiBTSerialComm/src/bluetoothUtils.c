/*
 * ============================================================================
 * Name        : bluetoothUtils.c
 * Author      : Brice DUCARDONNOY
 * Created on  : 28 janv. 2015
 * Version     :
 * Copyright   : Copyright © 2015 Brice DUCARDONNOY
 * Description : Utils functions for BlueTooth communication in C
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
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <sys/socket.h>
#include <string.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>

#include "Constants.h"
#include "Ttys/ttys.h"

int simpleScan(void) {
	printf("Enter in %s\n", __FUNCTION__);
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

/*! \brief Open a socket and wait for a connection request. The user has to close it then!
 *
 * @param ctx The context
 * @param timeout The timeout in s
 * @returns a File Descriptor of the connection done if a client has required a connection, -1 if nobody comes after a timeout
 */
int wait4connect(GlbCtx_t ctx, int timeout) {
	printf("Enter in %s\n", __FUNCTION__);
	struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
	char buf[1024] = { 0 };
	struct timeval timeoutConnect, timeoutWrite;

	socklen_t opt = sizeof(rem_addr);
	timeoutConnect.tv_sec = timeout;
	timeoutConnect.tv_usec = 0;
	timeoutWrite.tv_sec = 2;
	timeoutWrite.tv_usec = 0;

	// allocate socket
	if((ctx->sockFd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM)) == -1) {
		fprintf(stderr, "Error code %d on creating socket attempt::%s\n", errno, strerror(errno));
		return -1;
	}

	// bind socket to port 1 of the first available
	// local bluetooth adapter
	loc_addr.rc_family = AF_BLUETOOTH;
	loc_addr.rc_bdaddr = *BDADDR_ANY;
	loc_addr.rc_channel = (uint8_t) 1;
	if(bind(ctx->sockFd, (struct sockaddr *)&loc_addr, sizeof(loc_addr)) != 0) {
		fprintf(stderr, "Error code %d on bind attempt::%s\n", errno, strerror(errno));
		return -1;
	}

	 if(setsockopt (ctx->sockFd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeoutConnect, sizeof(timeoutConnect)) < 0) {
		 fprintf(stderr, "setsockopt failed. %d::%s\n", errno, strerror(errno));
		 return -1;
	 }

	 if(setsockopt (ctx->sockFd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeoutWrite, sizeof(timeoutWrite)) < 0) {
		 fprintf(stderr, "setsockopt2 failed. %d::%s\n", errno, strerror(errno));
		 return -1;
	 }

	// put socket into listening mode for 1 connection
	if(listen(ctx->sockFd, 1) == -1) {
		fprintf(stderr, "Error code %d on listen attempt::%s\n", errno, strerror(errno));
		return -1;
	}

	// accept one connection
	if((ctx->clienttFd = accept(ctx->sockFd, (struct sockaddr *)&rem_addr, &opt)) < 0) {
		fprintf(stderr, "Error code %d on accept attempt::%s\n", errno, strerror(errno));
		return -1;
	}

	ba2str( &rem_addr.rc_bdaddr, buf );
	fprintf(stderr, "accepted connection from %s\n", buf);
	memset(buf, 0, sizeof(buf));

	return 0;
}

/*! \brief Brief description.
 *         Brief description continued.
 *
 *  Detailed description starts here.
 */
int readAndRepeat(GlbCtx_t ctx) {
	char buf[1024] = { 0 };
	int bytes_read;
	printf("Enter in %s\n", __FUNCTION__);

	// read data from the client
	do {
		memset(buf, 0, sizeof(buf));
		bytes_read = read(ctx->clienttFd, buf, sizeof(buf) - 1);
		if( bytes_read > 0 ) {
			printf("received [%s]\n", buf);
			usleep(500000);// Half second
			printf("Write %d bytes\n", bytes_read);
//			if(write(ctx->clienttFd, buf, bytes_read) != bytes_read) {
			if(write(ctx->clienttFd, "patate\r\n", 8) != 8) {
				fprintf(stderr, "Failed to write: %d::%s\n", errno, strerror(errno));
			}
		}
	} while(strstr(buf, "EOC") == NULL);

	return 0;
}

int rfcommServer(void) {
	printf("Enter in %s\n", __FUNCTION__);
	struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
	char buf[1024] = { 0 };
	int socketFd, client, bytes_read;
	socklen_t opt = sizeof(rem_addr);

	// allocate socket
	socketFd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

	// bind socket to port 1 of the first available
	// local bluetooth adapter
	loc_addr.rc_family = AF_BLUETOOTH;
	loc_addr.rc_bdaddr = *BDADDR_ANY;
	loc_addr.rc_channel = (uint8_t) 1;
	bind(socketFd, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

	// put socket into listening mode
	listen(socketFd, 1);

	// accept one connection
	client = accept(socketFd, (struct sockaddr *)&rem_addr, &opt);

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
	close(socketFd);
	return 0;
}

int initAndTalkWithBTDevice(void) {
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
	if((ret = ttys_sendstring(ttys, "Hello world from RPi!!!\r\n")) != TTYS_OK) {
		fprintf(stderr, "Failed to write::%d returned\n", ret);
	}
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
