AUTOMAKE_OPTIONS=subdir-objects
#SUBDIRS=ttys
#AM_CFLAGS		= -I./ttys
#AM_CFLAGS		= -Wall -O2 -g
AM_CFLAGS		= -Wall -I$(top_srcdir)/dependencies/includes
AM_LDFLAGS  	= -L$(top_srcdir)/dependencies/libs -lbluetooth

bin_PROGRAMS=sewSerialComm

sewSerialComm_SOURCES= ttys/ttys.c RpiBTSerialComm.c

