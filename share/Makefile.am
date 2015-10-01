scriptsdir = $(prefix)/etc/init.d
scripts_SCRIPTS=checkInterfaceStatus.sh
#scripts_SCRIPTS=checkInterfaceStatus.sh sqdmlf2.sh
# Does the trick but looses the x flag because _DATA is more global. _SCRIPTS is for executable scripts so x flag is kept.
#scripts_DATA=checkInterfaceStatus.sh
#scripts_DATA=checkInterfaceStatus.sh check2...

LINKDIR=$(prefix)/etc/rc2.d
LINK=S01checkInterfaceStatus

install-data-hook:
	$(MKDIR_P) $(LINKDIR)
	rm -f $(LINKDIR)/$(LINK)
	$(LN_S) ../init.d/checkInterfaceStatus.sh $(LINKDIR)/$(LINK)

#$(LN_S) ../init.d/checkInterfaceStatus.sh $(LINK)
