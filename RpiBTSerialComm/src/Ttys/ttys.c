#include "../Ttys/ttys.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/serial.h>


// TODO BDY: update error return code to use errno and its message
// initialize ttys context...send back pointer of structure
// ttys interface context, or NULL if something goes wrong...
Ttys_context *ttys_init(void)
{
	Ttys_context *context;
	context=malloc(sizeof(Ttys_context)+4);
	if(!context)
	{
		perror("malloc ");
		return NULL;
	}
	context->iofdin=1;	// '1' is standard linux input...
	context->iofdout=0;	// '0' is standard linux output...
	context->connected=0;
	context->bauds=B9600;
	context->parity=PARITY_NONE;
	strcpy(context->devname,"/dev/ttyS0");
	return context;
}

int ttys_free(Ttys_context **context)
{
	Ttys ttys;
	if(!*context)
	{
		fprintf(stderr,"TTYS already freed!\n");
		return TTYS_NOK;
	}
	ttys=*context;
	if(ttys->connected)
		ttys_disconnect(ttys);
	free(*context);
	*context=NULL;
	return TTYS_OK;
}

int ttys_sendchar(Ttys_context *context,char c)
{
	int res;
	if(!context)
		return TTYS_NOK;
	ttys_connect(context);
	if(!context->connected)
	{
		fprintf(stderr,"Can't send char, not connected!!\n");
		return TTYS_CANT_SEND;
	}
	if(context->iofdout==-1)
		return TTYS_CANT_SEND;
	res=write(context->iofdout,&c,1);
	return (res==1)?TTYS_OK:TTYS_NOK;
}

int ttys_sendstring(Ttys_context *context,char *str)
{
	int res,sizeSent;
	if(!context)
		return TTYS_NOK;
	ttys_connect(context);
	if(!context->connected)
	{
		fprintf(stderr,"Can't send string, not connected!!\n");
		return TTYS_CANT_SEND;
	}
	if(context->iofdout==-1)
		return TTYS_CANT_SEND;
	sizeSent=0;
	do{
		res=write(context->iofdout,&str[sizeSent],strlen(str)-sizeSent);
		if(res>0)
			sizeSent+=res;
	}while(res!=-1 && sizeSent<strlen(str));
	return (res!=-1)?TTYS_OK:TTYS_NOK;
}

int ttys_sendstring_slow(Ttys_context *context,char *str,int time)
{
	if(!context)
		return TTYS_NOK;
	ttys_connect(context);
	if(!context->connected)
	{
		fprintf(stderr,"Can't send string, not connected!!\n");
		return TTYS_CANT_SEND;
	}
	if(context->iofdout==-1)
		return TTYS_CANT_SEND;
	do{
		write(context->iofdout,str,1);
		str++;
		usleep(time);
	}while(*str);
	return TTYS_OK;
}

int ttys_sendbuffer(Ttys_context *context,char *buff,int size)
{
	int res,sizeSent;
	if(!context)
		return TTYS_NOK;
	ttys_connect(context);
	if(!context->connected)
	{
		fprintf(stderr,"Can't send string, not connected!!\n");
		return TTYS_CANT_SEND;
	}
	if(context->iofdout==-1)
		return TTYS_CANT_SEND;
	sizeSent=0;
	do{
		res=write(context->iofdout,&buff[sizeSent],size-sizeSent);
		if(res>0)
			sizeSent+=res;
	}while(res!=-1 && sizeSent<size);
	return (res!=-1)?TTYS_OK:TTYS_NOK;
}

int ttys_ischar(Ttys_context *context)
{
	fd_set read_fds;
	struct timeval tv;
	int res=0;

	if(!context)
		return TTYS_NOK;
	ttys_connect(context);
	if(!context->connected)
	{
		fprintf(stderr,"Can't send string, not connected!!\n");
		return TTYS_CANT_SEND;
	}
	tv.tv_sec=0;
	tv.tv_usec=1; // no wait
	FD_ZERO(&read_fds);
	FD_SET(context->iofdin,&read_fds);

	res=select(FD_SETSIZE,&read_fds,(fd_set *)NULL,(fd_set *)NULL,&tv);

	if(res>0)
		return TTYS_OK;
	return TTYS_GET_NOTHING;
}

int ttys_waitChar(Ttys_context *context,int timeout)
{
	int tst;
	fd_set read_fds;
	struct timeval tv;
	int res=0;

	if(!context)
		return TTYS_NOK;
	ttys_connect(context);
	if(!context->connected)
	{
		fprintf(stderr,"Can't send string, not connected!!\n");
		return TTYS_CANT_SEND;
	}
	// already something in buffer ?
	tv.tv_sec=timeout/1000;
	tv.tv_usec=(timeout%1000)*1000;
	FD_ZERO(&read_fds);
	FD_SET(context->iofdin,&read_fds);

	if(ioctl(context->iofdin,FIONREAD,&tst)!=-1 && tst)
	{
		return TTYS_OK;
	}
	res=select(FD_SETSIZE,&read_fds,(fd_set *)NULL,(fd_set *)NULL,&tv);
	if(res>0)
		return TTYS_OK;
	return TTYS_GET_NOTHING;
}

int ttys_getchar(Ttys_context *context,char *c)
{
	int siz=-1;
	if(!context)
		return TTYS_NOK;
	ttys_connect(context);
	if(!context->connected)
	{
		fprintf(stderr,"Can't read char, not connected!!\n");
		return TTYS_CANT_GET;
	}
	if(context->iofdin==-1)
		return TTYS_CANT_GET;
	if(ttys_ischar(context)==TTYS_GET_NOTHING)
		return TTYS_GET_NOTHING;
	siz=read(context->iofdin,c,1);
	if(siz==1)
		return TTYS_OK;
	if(!siz)
		return TTYS_GET_NOTHING;
	return TTYS_CANT_GET;
}

int ttys_getstring(Ttys_context *context,char *str,int len,int timeout1,int timeout2)
{
	fd_set read_fds;
	struct timeval tv;
	int res,siztot,tst,siz;
	if(!context)
		return TTYS_NOK;
	ttys_connect(context);
	if(!context->connected)
	{
		fprintf(stderr,"Can't read string, not connected!!\n");
		return TTYS_CANT_GET;
	}
	if(context->iofdin==-1)
		return TTYS_CANT_GET;
	tv.tv_sec=timeout1/1000;
	tv.tv_usec=(timeout1%1000)*1000;
	FD_ZERO(&read_fds);
	FD_SET(context->iofdin,&read_fds);
	res=select(FD_SETSIZE,&read_fds,(fd_set *)NULL,(fd_set *)NULL,&tv);
	if(res==-1)
		return TTYS_NOK;
	if(res<=0)
		return 0;
	siztot=0;
	do{
		int size2read;
		if(siztot)
		{
			tv.tv_sec=timeout2/1000;
			tv.tv_usec=(timeout2%1000)*1000;
			FD_ZERO(&read_fds);
			FD_SET(context->iofdin,&read_fds);
			res=select(FD_SETSIZE,&read_fds,(fd_set *)NULL,(fd_set *)NULL,&tv);
			if(res==-1)
				return TTYS_NOK;
			if(res<=0)
				break;
		}
		if(ioctl(context->iofdin,FIONREAD,&tst)==-1 || !tst)
			break;
		size2read=(tst>(len-siztot))?len-siztot:tst;
		siz=read(context->iofdin,&str[siztot],size2read);
		if(siz==-1)
			return TTYS_NOK;
		if(siz>0)
			siztot+=siz;
	}while(siz && siztot<len);

	str[siztot]=0;
	return siztot;
}

int ttys_getstringUntilKey(Ttys_context *context,char *str,int len,int timeout1,int timeout2,char key)
{
	fd_set read_fds;
	struct timeval tv;
	int res,siztot,tst,siz;
	if(!context)
		return TTYS_NOK;
	ttys_connect(context);
	if(!context->connected)
	{
		fprintf(stderr,"Can't read string, not connected!!\n");
		return TTYS_CANT_GET;
	}
	if(context->iofdin==-1)
		return TTYS_CANT_GET;
	tv.tv_sec=timeout1/1000;
	tv.tv_usec=(timeout1%1000)*1000;
	FD_ZERO(&read_fds);
	FD_SET(context->iofdin,&read_fds);
	res=select(FD_SETSIZE,&read_fds,(fd_set *)NULL,(fd_set *)NULL,&tv);
	if(res==-1)
		return TTYS_NOK;
	if(res<=0)
		return 0;
	siztot=0;
	do{
		int size2read;
		if(siztot)
		{
			tv.tv_sec=timeout2/1000;
			tv.tv_usec=(timeout2%1000)*1000;
			FD_ZERO(&read_fds);
			FD_SET(context->iofdin,&read_fds);
			res=select(FD_SETSIZE,&read_fds,(fd_set *)NULL,(fd_set *)NULL,&tv);
			if(res==-1)
				return TTYS_NOK;
			if(res<=0)
				break;
		}
		if(ioctl(context->iofdin,FIONREAD,&tst)==-1 || !tst)
			break;
		size2read=(tst>(len-siztot))?len-siztot:tst;
		siz=read(context->iofdin,&str[siztot],size2read);
		if(siz==-1)
			return TTYS_NOK;
		if(siz>0)
			siztot+=siz;
		if(!siztot)
		{
			*str=0;
			return 0;
		}
	}while(siz && str[siztot-1]!=key && siztot<len);
	str[siztot]=0;
	return siztot;
}

int ttys_empty_input_buffer(Ttys_context *context)
{
	char c;
	if(!context)
		return TTYS_NOK;
	ttys_connect(context);
	if(!context->connected)
	{
		fprintf(stderr,"Can't empty input buffer, not connected!!\n");
		return TTYS_CANT_GET;
	}
	tcflush(context->iofdin, TCIFLUSH);
	while(ttys_getchar(context,&c)==TTYS_OK);
	return TTYS_OK;
}

int ttys_configure_dev(Ttys_context *context,struct termios *configttys)
{

	// modify serial config...
	configttys->c_lflag=0;
	configttys->c_oflag=0;
	// ignore break
	configttys->c_iflag=IGNBRK;
	// no XON/XOFF
	configttys->c_iflag&=~(IXON|IXOFF|IXANY);
	// dont touch my input stream...
	configttys->c_iflag&=~(ISTRIP|INLCR|IGNCR|ICRNL|IUCLC);
	// dont touch output stream too...
	configttys->c_oflag&=~(OLCUC|ONLCR|OCRNL|ONOCR|ONLRET|OFILL);
	configttys->c_cc[VERASE]=8;	// erase
	configttys->c_cc[VKILL]=24;	// kill
	configttys->c_cc[VINTR]=3;	// interrupt
	// 1 char only
	configttys->c_cc[VMIN]=1;
	// we want to reaceive something
	configttys->c_cflag|=CREAD;
	// no hard controler
	configttys->c_cflag&=~CRTSCTS;
	// data avalailable now
	configttys->c_cc[VTIME]=0;
	// 8 data bits, no parity
	configttys->c_cflag&=~(PARENB|CSIZE);
	configttys->c_cflag|=CS8;
	if(context->parity!=PARITY_NONE)
		configttys->c_cflag|=PARENB;
	if(context->parity==PARITY_ODD)
		configttys->c_cflag|=PARODD;
	// ignore modem signals
	configttys->c_cflag|=CLOCAL;
	// 1 stop bit...
	configttys->c_cflag&=~CSTOPB;

	// no canonical mode, neither local echo...
	configttys->c_lflag&=~(ICANON|ECHO);

	cfsetispeed(configttys,context->bauds);
	cfsetospeed(configttys,context->bauds);
	return TTYS_OK;
}

int ttys_setParity(Ttys_context *context,int parity)
{
	int connected=0;
	if(!context)
		return TTYS_NOK;
	if(context->connected)
	{
		ttys_disconnect(context);
		connected=1;
	}

	context->parity=parity;

	if(connected)
		return ttys_connect(context);
	return TTYS_OK;
}

void ttys_setLowLatency(Ttys_context *context,int val)
{
	context->lowlatency=val; // ultraspeed powa
}

static int setLowLatency(int fd)
{
	int res;
	struct serial_struct kernel_serial_settings;

	// attempt to set low latency mode, but don't worry if we can't
	res=ioctl(fd,TIOCGSERIAL,&kernel_serial_settings);
	if(res<0)
	{
		return TTYS_NOK;
	}
	kernel_serial_settings.flags|=ASYNC_LOW_LATENCY;
	ioctl(fd,TIOCSSERIAL,&kernel_serial_settings);
	return TTYS_OK;
}

int ttys_connect(Ttys_context *context)
{
	struct termios configttys_in;
	struct termios configttys_out;

	if(!context)
		return TTYS_NOK;
	if(context->connected)
	{
		return TTYS_OK;
	}
	// open devices...
	if((context->iofdin=open(context->devname,O_RDONLY|O_NONBLOCK))==-1)
	{
		perror("connect input ");
		return TTYS_CANTCONNECT;
	}
	if((context->iofdout=open(context->devname,O_WRONLY))==-1)
	{
		perror("connect output ");
		close(context->iofdin);
		return TTYS_CANTCONNECT;
	}

	// backup current serial config...
	tcgetattr(context->iofdin,&context->configttys_in);
	tcgetattr(context->iofdout,&context->configttys_out);

	// take current config...
	tcgetattr(context->iofdin,&configttys_in);
	// modify it...
	ttys_configure_dev(context,&configttys_in);
	// and save it...
	tcsetattr(context->iofdin,TCSANOW,&configttys_in);

	// idem for output config...
	tcgetattr(context->iofdout,&configttys_out);
	ttys_configure_dev(context,&configttys_out);
	tcsetattr(context->iofdout,TCSANOW,&configttys_out);

	// low latency ?
	if(context->lowlatency)
	{
		setLowLatency(context->iofdin);
		setLowLatency(context->iofdout);
	}

	context->connected=1;
	return TTYS_OK;
}

int ttys_disconnect(Ttys_context *context)
{
	if(!context)
		return TTYS_NOK;
	if(!context->connected)
	{
		fprintf(stderr,"Already disconnected!!\n");
		return TTYS_ALREADY_DISCONNECT;
	}
	if(context->iofdout!=-1)
	{
		// restore old serial output config...
		tcsetattr(context->iofdout,TCSANOW,&context->configttys_out);
		close(context->iofdout);
		context->iofdout=-1;
	}
	if(context->iofdin!=-1)
	{
		// restore old serial input config...
		tcsetattr(context->iofdin,TCSANOW,&context->configttys_in);
		close(context->iofdin);
		context->iofdin=-1;
	}
	context->connected=0;
	return TTYS_OK;
}

int ttys_setbauds(Ttys_context *context,int bauds)
{
	int connected=0;
	if(!context)
		return TTYS_NOK;
	switch(bauds) // 'I want my lut!'
	{
		case 50:
			context->bauds=B50;
			break;
		case 75:
			context->bauds=B75;
			break;
		case 110:
			context->bauds=B110;
			break;
		case 134:
			context->bauds=B134;
			break;
		case 150:
			context->bauds=B150;
			break;
		case 200:
			context->bauds=B200;
			break;
		case 300:
			context->bauds=B300;
			break;
		case 600:
			context->bauds=B600;
			break;
		case 1200:
			context->bauds=B1200;
			break;
		case 1800:
			context->bauds=B1800;
			break;
		case 2400:
			context->bauds=B2400;
			break;
		case 4800:
			context->bauds=B4800;
			break;
		case 9600:
			context->bauds=B9600;
			break;
		case 19200:
			context->bauds=B19200;
			break;
		case 38400:
			context->bauds=B38400;
			break;
		case 57600:
			context->bauds=B57600;
			break;
		case 115200:
			context->bauds=B115200;
			break;
		case 230400:
			context->bauds=B230400;
			break;
		default:
			fprintf(stderr,"Invalid Bauds!!\n");
			return TTYS_INVALID_BAUDS;
	}
	if(context->connected)
	{
		connected=1;
		ttys_disconnect(context);
	}
	if(connected)
		return ttys_connect(context);
	return TTYS_OK;
}

int ttys_setdevname(Ttys_context *context,char *filename)
{
	int connected=0;
	struct stat statfile;
	if(stat(filename,&statfile)==-1)
	{
		perror("setdevname ");
		return TTYS_INVALID_DEV;
	}
	if(strlen(filename)>=512)
		return TTYS_INVALID_DEV;
	strcpy(context->devname,filename);
	if(context->connected)
	{
		connected=1;
		ttys_disconnect(context);
	}
	if(connected)
		return ttys_connect(context);
	return TTYS_OK;
}




