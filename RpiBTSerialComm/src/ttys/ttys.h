
#ifndef __TTYS_H__
#define __TTYS_H__

#include <termios.h>

#define TTYS_OK					1
#define TTYS_NOK				0
#define TTYS_CANTCONNECT		-1
#define TTYS_INVALID_BAUDS		-2
#define TTYS_INVALID_DEV		-3
#define TTYS_ALREADY_CONNECT	-4
#define TTYS_ALREADY_DISCONNECT	-5
#define TTYS_CANT_SEND			-6
#define TTYS_CANT_GET			-7
#define TTYS_GET_NOTHING		-8

typedef struct{
	int iofdout; // output fd
	int iofdin; // input fd...
	int connected;
	speed_t bauds;
	int parity;
	char devname[512];
	struct termios configttys_in;
	struct termios configttys_out;
	int lowlatency;
}Ttys_context;
typedef Ttys_context* Ttys;

#define PARITY_NONE	0
#define PARITY_ODD	1
#define PARITY_EVEN	2

// declaration : do : Ttys ttys;

// initialize and get ttys context... ttys=ttys_init();
extern Ttys_context *ttys_init(void);
// free and destroy ttys context... ttys_free(&ttys);
extern int ttys_free(Ttys_context **context);
// configure speed... ttys_setbauds(ttys,9600);
extern int ttys_setbauds(Ttys_context *context,int bauds);
// configure device name... ttys_setdevname(ttys,"/dev/ttyS0");
extern int ttys_setdevname(Ttys_context *context,char *filename);
// configure parity...
extern int ttys_setParity(Ttys_context *context,int parity);
// to connect... ttys_connect(ttys);
extern int ttys_connect(Ttys_context *context);
// to disconnect... ttys_disconnect(ttys);
extern int ttys_disconnect(Ttys_context *context);
// send a solo char... ttys_sendchar(ttys,'c');
extern int ttys_sendchar(Ttys_context *context,char c);
// send a string... ttys_sendstring(ttys,"Hello,world!\r\n");
extern int ttys_sendstring(Ttys_context *context,char *str);
// slow down kid...
extern int ttys_sendstring_slow(Ttys_context *context,char *str,int time);
// send buffer...
extern int ttys_sendbuffer(Ttys_context *context,char *buff,int size);
// tell if something is available at input stream : TTYS_OK:yes, TTYS_GET_NOTHING:no
extern int ttys_ischar(Ttys_context *context);
// tell in something is available at input stream and wait : TTYS_OK:yes, TTYS_GET_NOTHING:no with timeout in ms
extern int ttys_waitChar(Ttys_context *context,int timeout);
// to receive a solo char (Non Blocking)... ttys_getchar(ttys,&c);
extern int ttys_getchar(Ttys_context *context,char *c);
// to get a string... size=ttys_getstring(ttys,buffer,127,1000,20);
// firest timeout wait for first char, 2nd is counted after 1st char received between each char...
extern int ttys_getstring(Ttys_context *context,char *str,int len,int timeout1,int timeout2);
// same with escape key
extern int ttys_getstringUntilKey(Ttys_context *context,char *str,int len,int timeout1,int timeout2,char key);
// empty input buffer...
extern int ttys_empty_input_buffer(Ttys_context *context);
// try to put serial in low latency mode
extern void ttys_setLowLatency(Ttys_context *context,int val);

/*
************* EXAMPLE *********************
#include <stdio.h>
#include <unistd.h>
#include "ttys.h"

int main(int argc,char *argv[])
{
	char buffer[128];
	Ttys ttys;

	ttys=ttys_init();
	ttys_setbauds(ttys,9600);
	ttys_setdevname(ttys,"/dev/ttyS0");
	ttys_connect(ttys);
	ttys_sendstring(ttys,"Hello,world!\r\n");
	ttys_empty_input_buffer(ttys);
	*buffer=0;
	if(ttys_getstring(ttys,buffer,127,1000,20)>0)
		printf("%s\n",buffer);
	
	if(ttys_getstringUntilKey(ttys,buffer,127,1000,20,'\n')>0)
		printf("%s\n",buffer);

	ttys_disconnect(ttys);
	ttys_free(&ttys);

	return 0;
}
*/

#endif
