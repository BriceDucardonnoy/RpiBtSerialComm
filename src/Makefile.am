AUTOMAKE_OPTIONS=subdir-objects
#SUBDIRS=ttys
#AM_CFLAGS		= -I./ttys
#AM_CFLAGS		= -Wall -O2 -g

AM_CFLAGS		= -Wall -I$(top_srcdir)/dependencies/includes
AM_CFLAGS		+= -O0 -g
AM_LDFLAGS  	= -L$(top_srcdir)/dependencies/libs -lbluetooth -liw

#bin_PROGRAMS=sewSerialComm testProtocol
bin_PROGRAMS=sewSerialComm

sewSerialComm_SOURCES= \
	Ttys/ttys.c \
	Network/networkManagement.c \
	Network/wifiTools.c \
	bluetoothUtils.c \
	communicationProtocol.c \
	RpiBTSerialComm.c


#testProtocol_SOURCES= communicationProtocol.c
