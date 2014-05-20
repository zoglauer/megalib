#----------------------------------------------------------------
#
#  Makefile
#
#  Switches into the subdirectories of the source path
#
#  Author: Andreas Zoglauer
#
#----------------------------------------------------------------


#----------------------------------------------------------------
# Commands:
#

#.SILENT:



#------------------------------------------------------------------------------
# Global:

glo:
	$(MAKE) -C global

clean_glomisc:
	@$(MAKE) clean_misc -C global

clean_glogui:
	@$(MAKE) clean_gui -C global


#------------------------------------------------------------------------------
# Geomega:

geo:
	$(MAKE) -C geomega

geolib:
	$(MAKE) lib -C geomega

clean_geo:
	@$(MAKE) clean -C geomega


#------------------------------------------------------------------------------
# Megalyze:

meg:
	@$(MAKE) -C megalyze

meglib:
	@$(MAKE) -r lib -C megalyze

clean_meg:
	@$(MAKE) clean -C megalyze


#------------------------------------------------------------------------------
# Fretalon:

fre:
	@$(MAKE) -C fretalon

frelib:
	@$(MAKE) -r lib -C fretalon

clean_fre:
	@$(MAKE) clean -C fretalon


#------------------------------------------------------------------------------
# spectralyze:

spe:
	@$(MAKE) -C spectralyze
	
spelib:
	@$(MAKE) lib -C spectralyze

clean_spe:
	@$(MAKE) clean -C spectralyze


#------------------------------------------------------------------------------
# Mimrec:

mim:
	@$(MAKE) -C mimrec

mimlib:
	@$(MAKE) lib -C mimrec

clean_mim:
	@$(MAKE) clean -C mimrec


#------------------------------------------------------------------------------
# Realta

rea:
	@$(MAKE) -C realta

clean_rea:
	@$(MAKE) clean -C realta


#------------------------------------------------------------------------------
# Revan:

rev:
	@$(MAKE) -C revan

revlib:
	@$(MAKE) lib -C revan

clean_rev:
	@$(MAKE) clean -C revan


#------------------------------------------------------------------------------
# Response:

res:
	@$(MAKE) -C response

reslib:
	@$(MAKE) lib -C response

clean_res:
	@$(MAKE) clean -C response


#------------------------------------------------------------------------------
# Herty:

her:
	@if test -d herty; then $(MAKE) -C herty; fi

herlib:
	@if test -d herty; then $(MAKE) lib -C herty; fi

clean_her:
	@if test -d herty; then $(MAKE) clean -C herty; fi


#------------------------------------------------------------------------------
# Sivan:

siv:
	@$(MAKE) -C sivan

sivlib:
	@$(MAKE) lib -C sivan

clean_siv:
	@$(MAKE) clean -C sivan


#------------------------------------------------------------------------------
# Cosima:

cos:
ifeq "$(strip $(GEANT4INSTALLED))" "0"
	@$(MAKE) -C cosima
endif

clean_cos:
ifeq "$(strip $(GEANT4INSTALLED))" "0"
	@$(MAKE) clean -C cosima -f Makefile
endif


#------------------------------------------------------------------------------
# Eview:

evi:
	@$(MAKE) -C eview

clean_evi:
	@$(MAKE) clean -C eview


#------------------------------------------------------------------------------
# Miworks:

miw:
	@if test -d miworks; then $(MAKE) -C miworks; fi

clean_miw:
	@if test -d miworks; then $(MAKE) clean -C miworks; fi


#------------------------------------------------------------------------------
# BeamMonitor:

bm:
	@if test -d beammonitor; then $(MAKE) -C beammonitor; fi

clean_bm:
	@if test -d beammonitor; then $(MAKE) clean -C beammonitor; fi


#------------------------------------------------------------------------------
# Calibration:

cal:
	@if test -d calibration; then $(MAKE) -C calibration; fi

clean_cal:
	@if test -d calibration; then $(MAKE) clean -C calibration; fi


#------------------------------------------------------------------------------
# Addon:

add:
	@$(MAKE) -C addon

clean_add:
	@$(MAKE) clean -C addon


#------------------------------------------------------------------------------
# All

link:
	@$(MAKE) link -C global
	@$(MAKE) link -C geomega
	@$(MAKE) link -C mimrec
	@$(MAKE) link -C realta
	@$(MAKE) link -C revan
	@$(MAKE) link -C sivan
	@$(MAKE) link -C eview
	@$(MAKE) link -C spectralyze
	@$(MAKE) link -C fretalon
	@$(MAKE) link -C response
ifeq "$(strip $(GEANT4INSTALLED))" "0"
	@$(MAKE) link -C cosima
endif
	@if test -d miworks; then $(MAKE) link -C miworks; fi
	@if test -d megalyze; then $(MAKE) link -C megalyze; fi
	@if test -d herty; then $(MAKE) link -C herty; fi
	@if test -d beammonitor; then $(MAKE) link -C beammonitor; fi
	@if test -d calibration; then $(MAKE) link -C calibration; fi

all:
	@$(MAKE) -C global
	@$(MAKE) -C geomega
	@$(MAKE) -C mimrec
	@$(MAKE) -C realta
	@$(MAKE) -C revan
	@$(MAKE) -C sivan
	@$(MAKE) -C eview
	@$(MAKE) -C spectralyze
	@$(MAKE) -C fretalon
	@$(MAKE) -C response
	@$(MAKE) -C addon
ifeq "$(strip $(GEANT4INSTALLED))" "0"
	@$(MAKE) -C cosima
endif
	@if test -d miworks; then $(MAKE) -C miworks; fi
	@if test -d megalyze; then $(MAKE) -C megalyze; fi
	@if test -d herty; then $(MAKE) -C herty; fi
	@if test -d beammonitor; then $(MAKE) -C beammonitor; fi
	@if test -d calibration; then $(MAKE) -C calibration; fi

clean:
	@$(MAKE) clean -C global
	@$(MAKE) clean -C geomega
	@$(MAKE) clean -C mimrec
	@$(MAKE) clean -C realta
	@$(MAKE) clean -C revan
	@$(MAKE) clean -C sivan
	@$(MAKE) clean -C eview
	@$(MAKE) clean -C spectralyze
	@$(MAKE) clean -C fretalon
	@$(MAKE) clean -C response
	@$(MAKE) clean -C addon
	@$(MAKE) clean -C cosima -f Makefile
	@rm -f *~
	@rm -f \#*
	@if test -d miworks; then $(MAKE) clean -C miworks; fi
	@if test -d megalyze; then $(MAKE) clean -C megalyze; fi
	@if test -d herty; then $(MAKE) clean -C herty; fi
	@if test -d beammonitor; then $(MAKE) clean -C beammonitor; fi
	@if test -d calibration; then $(MAKE) clean -C calibration; fi



#
#----------------------------------------------------------------



