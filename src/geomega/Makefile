#----------------------------------------------------------------
#
#  Makefile for MEGAlib / geomega
#
#  Author: Andreas Zoglauer
#
#----------------------------------------------------------------



#----------------------------------------------------------------
# Definitions:
#

PRG				= $(BN)/geomega

MOBJS     = $(LB)/MGeomegaMain.o

GOBJS     = $(LB)/MInterfaceGeomega.o \
						$(LB)/MGUIGeomegaMain.o \
						$(LB)/MGUIMGeant.o \
						$(LB)/MGUIPosition.o \

LOBJS     = $(LB)/MSettingsGeomega.o \
						$(LB)/MDGeometry.o \
						$(LB)/MDGeometryQuest.o \
						$(LB)/MDVolume.o \
						$(LB)/MDVolumeSequence.o \
						$(LB)/MDDetector.o \
						$(LB)/MDACS.o \
						$(LB)/MDAngerCamera.o \
						$(LB)/MDCalorimeter.o \
						$(LB)/MDStrip2D.o \
						$(LB)/MDStrip3D.o \
						$(LB)/MDStrip3DDirectional.o \
						$(LB)/MDDriftChamber.o \
						$(LB)/MDVoxel3D.o \
						$(LB)/MDShape.o \
						$(LB)/MDShapeBRIK.o \
						$(LB)/MDShapeSPHE.o \
						$(LB)/MDShapeTUBS.o \
						$(LB)/MDShapeTRD1.o \
						$(LB)/MDShapeTRD2.o \
						$(LB)/MDShapeTRAP.o \
						$(LB)/MDShapeGTRA.o \
						$(LB)/MDShapePCON.o \
						$(LB)/MDShapePGON.o \
						$(LB)/MDShapeCONE.o \
						$(LB)/MDShapeCONS.o \
						$(LB)/MDMaterial.o \
						$(LB)/MDMaterialComponent.o \
						$(LB)/MDSystem.o \
						$(LB)/MDTrigger.o \
						$(LB)/MDTriggerUnit.o \
						$(LB)/MDVector.o \
						$(LB)/MDDebugInfo.o \
						$(LB)/MDGrid.o \
						$(LB)/MDGridPoint.o \
						$(LB)/MDGridPointCollection.o \


SGOBJ = $(LB)/libGeomegaGui.$(DLL)
SLOBJ = $(LB)/libGeomega.$(DLL)

#----------------------------------------------------------------
# Commands:
#

all: $(PRG)

lib: $(SLOBJ) 

link:
		@$(LINK) $(shell pwd)/inc/*.h $(IN)
		@rm -f $(IN)/LinkDef.h


clean:
		@rm -f src/*~
		@rm -f src/\#*
		@rm -f inc/*~
		@rm -f inc/\#*
		@rm -f *~
		@rm -f \#*
		@rm -f $(PRG)
		@rm -f $(MOBJS) $(GOBJS) $(LOBJS) $(SGOBJ) $(SLOBJ)


#----------------------------------------------------------------
# Dependencies:
#

$(MOBJS): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(SLOBJ): $(LOBJS)
	@echo "Linking $(subst $(LB)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(LOBJS) $(GLIBS) $(LIBS) -o $(SLOBJ)

$(SGOBJ): $(GOBJS) $(LOBJS)
	@echo "Linking $(subst $(LB)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(GOBJS) $(GLIBS) $(LIBS) -o $(SGOBJ)

$(LOBJS): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(GOBJS): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(PRG): $(SGOBJ) $(SLOBJ) $(MOBJS)
	@echo "Linking $(subst $(BN)/,,$@) ..."
	$(LD) $(LDFLAGS) $(GEOMEGALIBS) $(GLIBS) $(LIBS) -o $(PRG)

#
#----------------------------------------------------------------



