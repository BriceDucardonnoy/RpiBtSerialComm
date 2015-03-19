/*
 * ============================================================================
 * Name        : RpiBTSerialComm.h
 * Author      : Brice DUCARDONNOY
 * Created on  : 19 mars 2015
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

#ifndef RPIBTSERIALCOMM_H_
#define RPIBTSERIALCOMM_H_

//typedef void * func_pointer(int);
/* Global structure */
typedef struct {
	/* BlueTooth communication */
	int sockFd;/* FD for BlueTooth serial communication */
	int clienttFd;/* Another FD... */
	/* Communication protocol */
//	void * (*commMethods[NB_COMMANDS]) (void *params);/* Function pointer array */
	/* WiFi */
	wireless_scan_head *wHead;/* Scan array response */
} GlbCtx;
typedef GlbCtx * GlbCtx_t;

typedef struct stArgs {
	GlbCtx_t ctx;
	uint8_t *array;
	int arrayLength;
} stArgs, *stArgs_t;

//typedef struct commFunc {
//	int (*commMethods) (stArgs_t args);
//	// ...
//} commFunc, *commFunc_t;


extern GlbCtx_t initContext(void);
extern void destroyContext(GlbCtx_t ctx);
extern int callFunction(int funcCode, stArgs_t args);

#endif /* RPIBTSERIALCOMM_H_ */
