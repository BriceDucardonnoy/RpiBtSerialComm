dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(RpiBTSerialComm, 1.0)


AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()

AC_PROG_CC
AC_PROG_LN_S
AC_PROG_MKDIR_P

AC_CONFIG_FILES(Makefile \
				src/Makefile \
				share/Makefile \
				)
# Create a link from share/checkInterfaceStatus.sh to install/etc/init.d/checkNetworkStatusd
#AC_CONFIG_LINKS(install/etc/init.d/checkNetworkStatusd:share/checkInterfaceStatus.sh)
AC_OUTPUT
