scriptsdir = $(prefix)/etc/init.d
scripts_SCRIPTS=checkInterfaceStatus.sh monitorNetworkConnection
# Does the trick but looses the x flag because _DATA is more global. _SCRIPTS is for executable scripts so x flag is kept.
#scripts_DATA=checkInterfaceStatus.sh
#scripts_DATA=checkInterfaceStatus.sh check2...

LINKDIR_START=$(prefix)/etc/rc2.d
LINK_START=S03monitorNetworkConnection
LINKDIR_STOP=$(prefix)/etc/rc6.d
LINK_STOP=K01monitorNetworkConnection

install-data-hook:
	$(MKDIR_P) $(LINKDIR_START) $(LINKDIR_STOP)
	rm -f $(LINKDIR_START)/$(LINK_START) $(LINKDIR_STOP)/$(LINK_STOP)
	$(LN_S) ../init.d/monitorNetworkConnection $(LINKDIR_START)/$(LINK_START)
	$(LN_S) ../init.d/monitorNetworkConnection $(LINKDIR_STOP)/$(LINK_STOP)

#$(LN_S) ../init.d/checkInterfaceStatus.sh $(LINK)
