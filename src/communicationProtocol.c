/*
 * ============================================================================
 * Name        : communicationProtocol.c
 * Author      : Brice DUCARDONNOY
 * Created on  : 18 mars 2015
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
/*
 * Protocol V1
 * FE<header><cmd>[<parameters>]<CRC>FF
 * Max size is 255 bytes
 * - Header: 2 bytes
 * 		Version number: 1 byte
 * 		Byte count of <cmd> and <parameters>: 1 byte
 * - Command: ID of the command to execute. Depending of this one, parameters are present.
 * - CRC: 2 bytes from frame without FE and FF
 *
 * Example in hexa of DISCOVER_WIFI request in protocol V1:
 * FE-Version-SZ-CMD-CRC_MSB-CRC_LSB-FF
 * FE-01	 -01-00 -CRC_MSB-CRC_LSB-FF
 *
 * FIXME BDY: what if the CRC contains FE or FF
 **Byte stuffing**
 * FD => FD 00
 * FE => FD 01
 * FF => FD 02
 */
#include <stdio.h>
#include <netinet/in.h>

#include "constants.h"
#include "Network/wifiTools.h"
#include "communicationProtocol.h"

crc_t crcTable[256];
static void crcInit(void);
static void printMessage(uint8_t *message, int len);

int deserialize(GlbCtx_t ctx, unsigned char *rxData) {
	static int isInitialized = FALSE;
	int version = rxData[1];
	int sz;
	int crcInd;
	int cmd;
	int ret;

	if(isInitialized == FALSE) {
		printf("Initialize crc\n");
		isInitialized = TRUE;
		crcInit();
	}
	/* Valid integrity of frame */
	if(rxData[0] != 0xFE) {
		fprintf(stderr, "The start flag isn't present. Skip operation.\n");
		return EXIT_ABORT;
	}
	if(version == 1) {
		sz = rxData[2];
		cmd = rxData[3];

		crcInd = 3 + sz;// FE, version, size: <cmd> starts at 4th position
		if(rxData[2 + sz + 3] != 0xFF) {// +3: CRC(+2) and 0xFF(+1)
			fprintf(stderr, "The end flag isn't present at the good. Skip operation.\n");
			return EXIT_ABORT;
		}
		// Check CRC
		printf("crc ind is %d => CRC_MSB = 0x%02X and CRC_LSB = 0x%02X\n", crcInd, rxData[crcInd], rxData[crcInd + 1]);
		uint16_t rxCrc = (rxData[crcInd] << 8) | rxData[crcInd + 1];
		uint16_t calculatedCrc = calculateCrc16(&rxData[1], sz + 2);
		printf("crc calculated is 0x%04X and got in message is 0x%04X\n", calculatedCrc, htons(rxCrc));
		if(htons(rxCrc) != calculatedCrc) {
			printf("BAD CRC\n");
			return EXIT_FAILURE;
		}
		else {
			printf("GOOD CRC\n");
//			(*ctx->commMethods[0])(NULL);
//			ctx->wHead = (*ctx->commMethods[cmd])(NULL);
			stArgs_t args = malloc(sizeof(struct stArgs));
			args->ctx = ctx;
			args->array = rxData;
			args->arrayLength = sz + 3;// Because sz is at index 2
			ret = callFunction(cmd, args);
			args->ctx = NULL;
			free(args);
			return ret;
		}
		// TODO BDY: undo the byte stuffing if needed to translate the message
	}
	else {
		fprintf(stderr, "Version %u unknown. Skip operation.\n", rxData[1]);
		return EXIT_ABORT;
	}

	return EXIT_SUCCESS;
}

static void crcInit(void) {
	crc_t  remainder;
	int dividend;
	uint8_t bit;
	/*
	 * Compute the remainder of each possible dividend.
	 */
	for (dividend = 0; dividend < 256; ++dividend) {
		/*
		 * Start with the dividend followed by zeros.
		 */
		remainder = dividend << (WIDTH - 8);
		/*
		 * Perform modulo-2 division, a bit at a time.
		 */
		for (bit = 8; bit > 0; --bit) {
			/*
			 * Try to divide the current data bit.
			 */
			if (remainder & TOPBIT) {
				remainder = (remainder << 1) ^ POLYNOMIAL;
			}
			else {
				remainder = (remainder << 1);
			}
		}
		/*
		 * Store the result into the table.
		 */
		crcTable[dividend] = remainder;
	}

}

uint16_t calculateCrc16(uint8_t *message, int nBytes) {
	uint8_t uiData;
	crc_t remainder = 0;
	int byte;

	/*
	 * Divide the message by the polynomial, a byte at a time.
	 */
	for (byte = 0 ; byte < nBytes ; ++byte) {
		uiData = message[byte] ^ (remainder >> (WIDTH - 8));
		remainder = crcTable[uiData] ^ (remainder << 8);
	}

	/*
	 * The final remainder is the CRC.
	 */
	return (remainder);
}

static void printMessage(uint8_t *message, int len) {
	int i;
	printf("Message: ");
	for(i = 0 ; i < len ; i++) {
		printf("0x%02X ", message[i]);
	}
	printf("\n");
}

void testProtocol(GlbCtx_t ctx) {
	printf("Enter in %s\n", __FUNCTION__);
	crcInit();
	uint8_t pdu[3] = {1, 1, DISCOVER_WIFI};
	uint16_t crc = calculateCrc16(pdu, 3);
	printf("crc calculated for message is 0x%04X\n", crc);
	uint8_t message[7] = {0xFE, 1, 1, DISCOVER_WIFI, (uint8_t) (crc & 0x00FF), (uint8_t) (crc >> 8), 0xFF};

	printMessage(message, 7);// Displays the FF
//	printf("Message is %s\n", message);
	printf("1st\n");
	deserialize(ctx, message);
	printf("2nd\n");
	deserialize(ctx, message);

	printf("Clean context\n");
	destroyContext(ctx);
}

//int main(int argc, char **argv) {
//	printf("Start test of protocol\n");
//	return EXIT_SUCCESS;
//}
