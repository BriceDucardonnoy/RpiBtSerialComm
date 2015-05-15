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
 * =>
 * FE-ProtocolVersion-Size-CMD-Parameters-CRC16-FF
 * ------------------------------------------------
 * Max size is 255 bytes
 * Command ID can't be 0xFE or 0xFF (not managed yet because of optimistic code in serializeAndAnswer function)
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
 *
 * For CRC calcul, see http://introcs.cs.princeton.edu/java/51data/CRC16.java.html
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "constants.h"
#include "Network/wifiTools.h"
#include "communicationProtocol.h"

/* Communication */
//#define POLYNOMIAL 0xC0C1

#define MAX_PACKET_LENGTH 256

//static void crcInit(void);
static void printMessage(uint8_t *message, int len);
static int getStuffedMessageLength(uint8_t *stuffedMessage, int rawSz);
// Uses irreducible polynomial:  1 + x^2 + x^15 + x^16
static uint16_t crcTable[256] = {
	0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
	0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
	0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
	0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
	0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
	0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
	0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
	0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
	0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
	0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
	0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
	0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
	0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
	0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
	0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
	0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
	0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
	0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
	0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
	0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
	0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
	0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
	0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
	0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
	0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
	0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
	0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
	0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
	0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
	0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
	0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
	0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040,
};

/*
 * Functions to apply / remove the byte stuffing into the message
 */
uint8_t * unbyteStuffFrame(uint8_t *messageStuffed) {
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

uint8_t * byteStuffRawFrame(uint8_t *message, int rawSz) {
	uint8_t *messageStuffed = malloc(rawSz*2);// Size of packet is 1 byte so the stuffed packet in worst case can be more than rawSz*2 bytes
	int i, j;
//	int sz = message[1] == 1 ? message[2] + 3 + 3 : 0;
	/*
	 * If version is 1, message size is
	 * <declared sz> at index 2
	 * +					  3 (for message[0] and message[1] and message[2])
	 * + 					  3 (for 2 CRC bytes and 0xFF
	 */
//	printf("SZ = %d\n", sz);

	for(i = 0, j = 0 ; i < rawSz ; i++) {
		if(i == 0 || i == rawSz - 1 || // We don't apply byte stuffing on flags themselves
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
 * Functions to (de)serialize the clean message without byte stuffing and execute the command and answer
 */
int deserializeAndProcessCmd(glbCtx_t ctx, uint8_t *rxData) {
	printf("Enter in %s\n", __FUNCTION__);
	printMessage(rxData, getStuffedMessageLength(rxData, -1));
	uint8_t *message;
	int version;
	int sz;
	int crcInd;
	int cmd;
	int ret;

	message = unbyteStuffFrame(rxData);
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
	if(version == 0) {
		// Wild card. Remote is asking the protocol version.
		uint8_t ver[] = {0xFE, 0, PROTOCOL_VERSION, 0xFF};//, 13, 10};
		printf("Version asked. ");
		printMessage(ver, 4);
		if(write(ctx->clienttFd, &ver, 4) != 4) {
			fprintf(stderr, "Failed to write: %d::%s\n", errno, strerror(errno));
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
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
		uint16_t calculatedCrc = calculateCrc16(&message[1], sz + 2);// sz (:cmd + parameters) + protocol + size
		printf("crc calculated is 0x%04X and got in message is 0x%04X\n", calculatedCrc, rxCrc);
		if(rxCrc != calculatedCrc) {
			printf("BAD CRC\n");
			return EXIT_FAILURE;
		}
		else {
			printf("GOOD CRC\n");
//			(*ctx->commMethods[0])(NULL);
//			ctx->wHead = (*ctx->commMethods[cmd])(NULL);
			stArgs_t args = malloc(sizeof(struct stArgs));
			args->ctx = ctx;
			args->input = message;
			args->inputLength = 3 + sz + 3;// Because sz is at index 2
//			args->ctx->sockFd = stdout;
			ret = callFunction(cmd, args);
			if(args->output) {
				ret = serializeAndAnswer(args);
			}
			args->ctx = NULL;
			cleanArgs(args);
			free(args);
			return ret;
		}
	}
	fprintf(stderr, "Version %u unknown. Skip operation.\n", message[1]);
	return EXIT_ABORT;

	return EXIT_SUCCESS;
}

int serializeAndAnswer(stArgs_t args) {
	printf("Enter in %s\n", __FUNCTION__);
	int protocol = args->input[1];
	int smSz;
	int outSz = 1 + 2 + 1 + args->outputLength + 2 + 1;// FE + (Protocol Version + Sz) + CMD + output + CRC16 + FF
	uint16_t crc;
	uint8_t *stuffedMessage;
	uint8_t *fullOutput = malloc(outSz);

	fullOutput[0] = 0xFE;
	fullOutput[1] = protocol;

	if(protocol == 1) {
		fullOutput[2] = 1 + args->outputLength;// Size includes CMD + parameters (output in case of answer)
		fullOutput[3] = args->input[3];// Command
		memcpy(fullOutput + 4, args->output, args->outputLength);
		// CRC
		crc = htons(calculateCrc16(fullOutput + 1, args->outputLength + 3));// Output and (protocol and size and command)
		memcpy(fullOutput + 4 + args->outputLength, &crc, 2);
		fullOutput[4 + args->outputLength + 2] = 0xFF;// + 2: CRC
		// Full output is now completed => parse it to the byte stuffing
		stuffedMessage = byteStuffRawFrame(fullOutput, outSz);
		smSz = getStuffedMessageLength(stuffedMessage, outSz);
		printf("Raw answer (%d bytes): ", outSz);
		printMessage(fullOutput, outSz);
		printf("\nStuffed answer (%d bytes): ", smSz);
		printMessage(stuffedMessage, smSz);
		if(smSz < 0) {
			printf("No answer to write\n");
			if(stuffedMessage) free(stuffedMessage);
			free(fullOutput);
			return EXIT_ABORT;
		}
//		// Add \r\n because receiver device is expected it as end of response
//		smSz += 2;
//		stuffedMessage = realloc(stuffedMessage, smSz);
//		stuffedMessage[smSz - 2] = 13;// \r
//		stuffedMessage[smSz - 1] = 10;// \n
		//
		printf("Write answer\n");
		if(write(args->ctx->clienttFd, stuffedMessage, smSz) != smSz) {
			fprintf(stderr, "Failed to write: %d::%s\n", errno, strerror(errno));
		}
		if(stuffedMessage) free(stuffedMessage);
		free(fullOutput);
		return EXIT_SUCCESS;
	}
	fprintf(stderr, "Version %u unknown. Skip operation.\n", protocol);
	return EXIT_ABORT;
}

/*
 * \brief Calculates the CRC16 with polynome 0xC0C1
 * Calculates the CRC16 with polynome 0xC0C1. CRC gotten is already big-endian order
 */
uint16_t calculateCrc16(uint8_t *message, int nBytes) {
	uint16_t remainder = 0;
	int byte;

	// Divide the message by the polynomial, a byte at a time.
	for (byte = 0 ; byte < nBytes ; ++byte) {
		remainder = (remainder >> 8) ^ crcTable[(remainder ^ message[byte]) & 0xFF];
	}

	// The final remainder is the CRC.
	return (remainder);
}

/*
 * \brief Convenient method to display the content of a message in hexa
 */
static void printMessage(uint8_t *message, int len) {
	int i;
	if(len == -1) len = RX_BUFFER_SIZE;
	printf("Message: ");
	for(i = 0 ; i < len ; i++) {
		printf("0x%02X ", message[i]);
	}
	printf("\n");
}

static int getStuffedMessageLength(uint8_t *stuffedMessage, int rawSz) {
	int i = 0;
	if(rawSz == -1) rawSz = RX_BUFFER_SIZE;
	while(i < (rawSz * 2)) {// Size of packet is 1 byte so the stuffed packet in worst case can be more than rawSz*2 bytes
		if(stuffedMessage[i] == 0xFF) return ++i;// include this last byte
		i++;
	}
	return -1;
}

void testProtocol(glbCtx_t ctx) {
	printf("Enter in %s\n", __FUNCTION__);
//	crcInit();
	uint8_t pdu[3] = {1, 1, DISCOVER_WIFI};
	uint16_t crc = calculateCrc16(pdu, 3);
	printf("crc calculated for message is 0x%04X\n", crc);
	uint8_t rawMessage[] = {0xFE, 1, 1, DISCOVER_WIFI, (uint8_t) (crc >> 8), (uint8_t) (crc & 0x00FF), 0xFF};
	uint8_t *stuffedMessage;
	uint8_t *cleanMessage;

	printf("Raw discover WIFI");// rawMessage and clean/unstuffed Message should be the same
	printMessage(rawMessage, 7);// Displays the FF
	printf("Stuffed ");// Message as it should be receive from client
	stuffedMessage = byteStuffRawFrame(rawMessage, 7);
	int stuffedSz = getStuffedMessageLength(stuffedMessage, 7);
//	printf("Stuffed message size is %d\n", stuffedSz);
	printMessage(stuffedMessage, stuffedSz);
	printf("Unstuffed ");// Message as it should be processed in embed
	cleanMessage = unbyteStuffFrame(stuffedMessage);
	printMessage(cleanMessage, 7);

	printf("************************************************\n");

//	printf("Message is %s\n", message);
	printf("1st\n");
	deserializeAndProcessCmd(ctx, stuffedMessage);
	printf("------------------------------------------------\n");
	printf("2nd\n");
	deserializeAndProcessCmd(ctx, stuffedMessage);

	printf("Clean context\n");
	if(stuffedMessage) free(stuffedMessage);
	if(cleanMessage) free(cleanMessage);
	destroyContext(ctx);
}

void testNetwork(glbCtx_t ctx) {
	printf("Enter in %s\n", __FUNCTION__);
	uint8_t pdu[3] = {1, 1, GET_NETWORK};
	uint16_t crc = calculateCrc16(pdu, 3);
	printf("crc calculated for message is 0x%04X\n", crc);
	uint8_t rawMessage[] = {0xFE, 1, 1, GET_NETWORK, (uint8_t) (crc >> 8), (uint8_t) (crc & 0x00FF), 0xFF};
	uint8_t *stuffedMessage;
	uint8_t *cleanMessage;

	printf("Raw get network");// rawMessage and clean/unstuffed Message should be the same
	printMessage(rawMessage, 7);// Displays the FF
	printf("Stuffed ");// Message as it should be receive from client
	stuffedMessage = byteStuffRawFrame(rawMessage, 7);
	int stuffedSz = getStuffedMessageLength(stuffedMessage, 7);
	//	printf("Stuffed message size is %d\n", stuffedSz);
	printMessage(stuffedMessage, stuffedSz);
	printf("Unstuffed ");// Message as it should be processed in embed
	cleanMessage = unbyteStuffFrame(stuffedMessage);
	printMessage(cleanMessage, 7);

	printf("************************************************\n");

	//	printf("Message is %s\n", message);
	printf("1st\n");
	deserializeAndProcessCmd(ctx, stuffedMessage);
	printf("------------------------------------------------\n");
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
