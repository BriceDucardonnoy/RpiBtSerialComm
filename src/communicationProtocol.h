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

#ifndef COMMUNICATIONPROTOCOL_H_
#define COMMUNICATIONPROTOCOL_H_

extern uint8_t * rxRawFrame(uint8_t *messageStuffed);
extern uint8_t * txRawFrame(uint8_t *message);
extern int deserializeAndProcessCmd(glbCtx_t ctx, unsigned char *rxData);
extern int serializeAndAnswer(stArgs_t args);
extern uint16_t calculateCrc16(uint8_t *message, int nBytes);
extern void testProtocol(glbCtx_t ctx);

#endif /* COMMUNICATIONPROTOCOL_H_ */
