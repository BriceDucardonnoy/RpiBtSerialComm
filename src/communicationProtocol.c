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
 * Byte stuffing applied to the serialized packet
 * ----------------------------------------------
 * FD => FD 00
 * FE => FD 01
 * FF => FD 02
 */
#include <stdio.h>
#include <netinet/in.h>

#include "constants.h"
#include "Network/wifiTools.h"
#include "communicationProtocol.h"

#define MAX_PACKET_LENGTH 256

static crc_t crcTable[256];
static void crcInit(void);
static void printMessage(uint8_t *message, int len);
static int getStuffedMessageLength(uint8_t *stuffedMessage);

/*
 * Functions to apply / remove the byte stuffing into the message
 */
uint8_t * rxRawFrame(uint8_t *messageStuffed) {
	uint8_t *message = malloc(MAX_PACKET_LENGTH);// Size of packet is 1 byte so the unstuffed packet can't be more than 255 bytes
	int i, cur, j;

	for(i = 0, cur = 0, j = 0 ; i < MAX_PACKET_LENGTH ; i++) {
		if(messageStuffed[i] != 0xFD || i == MAX_PACKET_LENGTH - 1) {
			message[j++] = messageStuffed[cur++];
			continue;
		}
		// messageStuffed[i] == 0xFD
		switch(messageStuffed[i+1]) {
		case 0x00:
			message[j++] = 0xFD;
			break;
		case 0x01:
			message[j++] = 0xFE;
			break;
		case 0x02:
			message[j++] = 0xFF;
			break;
		default:
			fprintf(stderr, "Unrecognized byte stuffing\n");
			return NULL;
		};
		cur++;
	}
	return message;
}

uint8_t * txRawFrame(uint8_t *message) {
	uint8_t *messageStuffed = malloc(MAX_PACKET_LENGTH*2);// Size of packet is 1 byte so the stuffed packet in worst case can be more than 255*2 bytes
	int i, j;
	int sz = message[1] == 1 ? message[2] + 3 + 3 : 0;
	/*
	 * If version is 1, message size is
	 * <declared sz> at index 2
	 * +					  3 (for message[0] and message[1] and message[2])
	 * + 					  3 (for 2 CRC bytes and 0xFF
	 */
//	printf("SZ = %d\n", sz);

	for(i = 0, j = 0 ; i < sz ; i++) {
		if(i == 0 || i == sz - 1 || // We don't apply byte stuffing on flags themselves
				(message[i] != 0xFD && message[i] != 0xFE && message[i] != 0xFF)) {
			messageStuffed[j++] = message[i];
			continue;
		}
		// Byte stuffing to do
		switch(message[i]) {
		case 0xFD:
			messageStuffed[j++] = 0xFD;
			messageStuffed[j++] = 0x00;
			break;
		case 0xFE:
			messageStuffed[j++] = 0xFD;
			messageStuffed[j++] = 0x01;
			break;
		case 0xFF:
			messageStuffed[j++] = 0xFD;
			messageStuffed[j++] = 0x02;
			break;
		default:
			fprintf(stderr, "Impossible case\n");
			return NULL;
		};
	}

	return messageStuffed;
}

/*
 * Functions to (de)serialize the clean message without byte stuffing
 */
int deserializeAndProcessCmd(GlbCtx_t ctx, unsigned char *rxData) {
	static int isInitialized = FALSE;
	uint8_t *message;
	int version;
	int sz;
	int crcInd;
	int cmd;
	int ret;

	if(isInitialized == FALSE) {
		printf("Initialize crc\n");
		crcInit();
		isInitialized = TRUE;
	}
	message = rxRawFrame(rxData);
	version = message[1];
	if(!message) {
		fprintf(stderr, "Failed to remove byte stuffing\n");
		return EXIT_ABORT;
	}
	/* Valid integrity of frame */
	if(message[0] != 0xFE) {
		fprintf(stderr, "The start flag isn't present. Skip operation.\n");
		return EXIT_ABORT;
	}
	if(version == 1) {
		sz = message[2];
		cmd = message[3];

		crcInd = 3 + sz;// FE, version, size: <cmd> starts at 4th position
		if(message[2 + sz + 3] != 0xFF) {// +3: CRC(+2) and 0xFF(+1)
			fprintf(stderr, "The end flag isn't present at the good. Skip operation.\n");
			return EXIT_ABORT;
		}
		// Check CRC
		printf("crc ind is %d => CRC_MSB = 0x%02X and CRC_LSB = 0x%02X\n", crcInd, message[crcInd], message[crcInd + 1]);
		uint16_t rxCrc = (message[crcInd] << 8) | message[crcInd + 1];
		uint16_t calculatedCrc = calculateCrc16(&message[1], sz + 2);
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
			args->array = message;
			args->arrayLength = sz + 3 + 3;// Because sz is at index 2
			ret = callFunction(cmd, args);
			args->ctx = NULL;
			free(args);
			return ret;
		}
	}
	else {
		fprintf(stderr, "Version %u unknown. Skip operation.\n", message[1]);
		return EXIT_ABORT;
	}

	return EXIT_SUCCESS;
}

int serialize(uint8_t message) {
	// TODO BDY: NYI
	return 0;
}

static void crcInit(void) {
	crc_t  remainder;
	int dividend;
	uint8_t bit;
	// Compute the remainder of each possible dividend.
	for (dividend = 0; dividend < 256; ++dividend) {
		// Start with the dividend followed by zeros.
		remainder = dividend << (WIDTH - 8);
		// Perform modulo-2 division, a bit at a time.
		for (bit = 8; bit > 0; --bit) {
			// Try to divide the current data bit.
			if (remainder & TOPBIT) {
				remainder = (remainder << 1) ^ POLYNOMIAL;
			}
			else {
				remainder = (remainder << 1);
			}
		}
		// Store the result into the table.
		crcTable[dividend] = remainder;
	}

}

uint16_t calculateCrc16(uint8_t *message, int nBytes) {
	uint8_t uiData;
	crc_t remainder = 0;
	int byte;

	// Divide the message by the polynomial, a byte at a time.
	for (byte = 0 ; byte < nBytes ; ++byte) {
		uiData = message[byte] ^ (remainder >> (WIDTH - 8));
		remainder = crcTable[uiData] ^ (remainder << 8);
	}

	// The final remainder is the CRC.
	return (remainder);
}

/*
 * \brief Convenient method to display the content of a message in hexa
 */
static void printMessage(uint8_t *message, int len) {
	int i;
	printf("Message: ");
	for(i = 0 ; i < len ; i++) {
		printf("0x%02X ", message[i]);
	}
	printf("\n");
}

static int getStuffedMessageLength(uint8_t *stuffedMessage) {
	int i = 0;
	while(i < MAX_PACKET_LENGTH) {
		if(stuffedMessage[i] == 0xFF) return ++i;// include this last byte
		i++;
	}
	return -1;
}

void testProtocol(GlbCtx_t ctx) {
	printf("Enter in %s\n", __FUNCTION__);
	crcInit();
	uint8_t pdu[3] = {1, 1, DISCOVER_WIFI};
	uint16_t crc = calculateCrc16(pdu, 3);
	printf("crc calculated for message is 0x%04X\n", crc);
	uint8_t rawMessage[] = {0xFE, 1, 1, DISCOVER_WIFI, (uint8_t) (crc & 0x00FF), (uint8_t) (crc >> 8), 0xFF};
	uint8_t *stuffedMessage;
	uint8_t *cleanMessage;

	printf("Raw ");// rawMessage and clean/unstuffed Message should be the same
	printMessage(rawMessage, 7);// Displays the FF
	printf("Stuffed ");// Message as it should be receive from client
	stuffedMessage = txRawFrame(rawMessage);
	int stuffedSz = getStuffedMessageLength(stuffedMessage);
//	printf("Stuffed message size is %d\n", stuffedSz);
	printMessage(stuffedMessage, stuffedSz);
	printf("Unstuffed ");// Message as it should be processed in embed
	cleanMessage = rxRawFrame(stuffedMessage);
	printMessage(cleanMessage, 7);

	printf("************************************************\n");

//	printf("Message is %s\n", message);
	printf("1st\n");
	deserializeAndProcessCmd(ctx, stuffedMessage);
	printf("2nd\n");
	deserializeAndProcessCmd(ctx, stuffedMessage);

	printf("Clean context\n");
	if(stuffedMessage) free(stuffedMessage);
	if(cleanMessage) free(cleanMessage);
	destroyContext(ctx);
}

//int main(int argc, char **argv) {
//	printf("Start test of protocol\n");
//	return EXIT_SUCCESS;
//}
