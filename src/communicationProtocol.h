/*
 * ============================================================================
 * Name        : communicationProtocol.h
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
 */
#ifndef COMMUNICATIONPROTOCOL_H_
#define COMMUNICATIONPROTOCOL_H_

extern uint8_t * unbyteStuffFrame(uint8_t *messageStuffed);
extern uint8_t * byteStuffRawFrame(uint8_t *message, int rawSz);
extern int deserializeAndProcessCmd(glbCtx_t ctx, uint8_t *rxData);
extern int serializeAndAnswer(stArgs_t args);
extern uint16_t calculateCrc16(uint8_t *message, int nBytes);
extern void testProtocol(glbCtx_t ctx);

#endif /* COMMUNICATIONPROTOCOL_H_ */
