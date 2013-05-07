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
	@$(MAKE) -C herty

herlib:
	@$(MAKE) lib -C herty

clean_her:
	@$(MAKE) clean -C herty


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
	@$(MAKE) -C miworks

clean_miw:
	@$(MAKE) clean -C miworks


#------------------------------------------------------------------------------
# BeamMonitor:

bm:
	@$(MAKE) -C beammonitor

clean_bm:
	@$(MAKE) clean -C beammonitor


#------------------------------------------------------------------------------
# Calibration:

cal:
	@$(MAKE) -C calibration

clean_cal:
	@$(MAKE) clean -C calibration


#------------------------------------------------------------------------------
# Addon:

add:
	@$(MAKE) -C addon

clean_add:
	@$(MAKE) clean -C addon


#------------------------------------------------------------------------------
# All

link:
	$(MAKE) link -C global
	$(MAKE) link -C geomega
	$(MAKE) link -C mimrec
	$(MAKE) link -C realta
	$(MAKE) link -C revan
	$(MAKE) link -C sivan
	$(MAKE) link -C eview
	$(MAKE) link -C miworks
	$(MAKE) link -C megalyze
	$(MAKE) link -C spectralyze
	$(MAKE) link -C herty
	$(MAKE) link -C beammonitor
	$(MAKE) link -C calibration
	$(MAKE) link -C response

all:
	@$(MAKE) -C global
	@$(MAKE) -C geomega
	@$(MAKE) -C mimrec
	@$(MAKE) -C realta
	@$(MAKE) -C revan
	@$(MAKE) -C sivan
	@$(MAKE) -C eview
	@$(MAKE) -C miworks
	@$(MAKE) -C megalyze
	@$(MAKE) -C spectralyze
	@$(MAKE) -C herty
	@$(MAKE) -C beammonitor
	@$(MAKE) -C calibration
	@$(MAKE) -C response
	@$(MAKE) -C addon
ifeq "$(strip $(GEANT4INSTALLED))" "0"
	@$(MAKE) -C cosima
endif

clean:
	@$(MAKE) clean -C global
	@$(MAKE) clean -C geomega
	@$(MAKE) clean -C mimrec
	@$(MAKE) clean -C realta
	@$(MAKE) clean -C revan
	@$(MAKE) clean -C sivan
	@$(MAKE) clean -C eview
	@$(MAKE) clean -C miworks
	@$(MAKE) clean -C megalyze
	@$(MAKE) clean -C spectralyze
	@$(MAKE) clean -C herty
	@$(MAKE) clean -C beammonitor
	@$(MAKE) clean -C calibration
	@$(MAKE) clean -C response
	@$(MAKE) clean -C addon
	@$(MAKE) clean -C cosima -f Makefile
	@rm -f *~
	@rm -f \#*



#
#----------------------------------------------------------------



