/*
 * ============================================================================
 * Name        : bluetoothUtils.h
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

#ifndef BLUETOOTHUTILS_H_
#define BLUETOOTHUTILS_H_

extern int simpleScan(void);
extern int rfcommServer(void);
extern int initAndTalkWithBTDevice(void);
/*! \brief Open a socket and wait for a connection request. The user has to close it then!
 *
 * @param ctx The context
 * @param timeout The timeout in s
 * @returns a File Descriptor of the connection done if a client has required a connection, -1 if nobody comes after a timeout
 */
extern int wait4connect(GlbCtx_t ctx, int timeout);
/*! \brief Brief description.
 *         Brief description continued.
 *
 *  Detailed description starts here.
 */
extern int readAndRepeat(GlbCtx_t ctx);
#endif /* BLUETOOTHUTILS_H_ */
