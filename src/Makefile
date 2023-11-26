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

dict_glo:
	$(MAKE) dict -C global

clean_glomisc:
	@$(MAKE) clean_misc -C global

clean_glogui:
	@$(MAKE) clean_gui -C global

clean_gloneuralnet:
	@$(MAKE) clean_neuralnet -C global


#------------------------------------------------------------------------------
# Geomega:

geo:
	$(MAKE) -C geomega

geolib:
	$(MAKE) lib -C geomega

dict_geo:
	$(MAKE) dict -C geomega

clean_geo:
	@$(MAKE) clean -C geomega


#------------------------------------------------------------------------------
# Fretalon:

fre:
	@$(MAKE) -C fretalon

frelib:
	@$(MAKE) -r lib -C fretalon

dict_fre:
	@$(MAKE) dict -C fretalon

clean_fre:
	@$(MAKE) clean -C fretalon

fretalonframework:
	@$(MAKE) framework -C fretalon

clean_fretalonframework:
	@$(MAKE) clean_framework -C fretalon


#------------------------------------------------------------------------------
# spectralyze:

spe:
	@$(MAKE) -C spectralyze
	
spelib:
	@$(MAKE) lib -C spectralyze

dict_spe:
	@$(MAKE) dict -C spectralyze

clean_spe:
	@$(MAKE) clean -C spectralyze


#------------------------------------------------------------------------------
# Mimrec:

mim:
	@$(MAKE) -C mimrec

mimlib:
	@$(MAKE) lib -C mimrec

dict_mim:
	@$(MAKE) dict -C mimrec

clean_mim:
	@$(MAKE) clean -C mimrec


#------------------------------------------------------------------------------
# Realta

rea:
	@$(MAKE) -C realta

dict_rea:
	@$(MAKE) dict -C realta

clean_rea:
	@$(MAKE) clean -C realta


#------------------------------------------------------------------------------
# Revan:

rev:
	@$(MAKE) -C revan

revlib:
	@$(MAKE) lib -C revan

dict_rev:
	@$(MAKE) dict -C revan

clean_rev:
	@$(MAKE) clean -C revan


#------------------------------------------------------------------------------
# Response:

res:
	@$(MAKE) -C response

reslib:
	@$(MAKE) lib -C response

dict_res:
	@$(MAKE) dict -C response

clean_res:
	@$(MAKE) clean -C response


#------------------------------------------------------------------------------
# Sivan:

siv:
	@$(MAKE) -C sivan

sivlib:
	@$(MAKE) lib -C sivan

dict_siv:
	@$(MAKE) dict -C sivan

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

dict_evi:
	@$(MAKE) dict -C eview

clean_evi:
	@$(MAKE) clean -C eview


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

dict: dict_glo dict_geo dict_mim dict_rea dict_rev dict_siv dict_evi dict_spe dict_fre dict_res

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



#
#----------------------------------------------------------------



