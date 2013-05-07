#----------------------------------------------------------------
#
#  Makefile for Mimrec
#
#  Author: Andreas Zoglauer
#
#----------------------------------------------------------------



#----------------------------------------------------------------
# Definitions:
#

PRG = $(BN)/mimrec

MOBJS     = $(LB)/MMimrecMain.o

GOBJS     = $(LB)/MInterfaceMimrec.o \
						$(LB)/MGUIMimrecMain.o \
						$(LB)/MGUIHistogramData.o \
						$(LB)/MGUIARM.o \
						$(LB)/MGUIAnimation.o \
						$(LB)/MGUICoordinateSystem.o \
						$(LB)/MGUIEventSelection.o \
						$(LB)/MGUIImageDimensions.o \
						$(LB)/MGUIImageOptions.o \
						$(LB)/MGUIDeconvolution.o \
						$(LB)/MGUISignificance.o \
						$(LB)/MGUIMemory.o \
						$(LB)/MGUIResponseParameterPRM.o \
						$(LB)/MGUIResponseParameterGauss1D.o \
						$(LB)/MGUIResponseSelection.o \
						$(LB)/MGUIPolarization.o \


LOBJS     = $(LB)/MSettingsMimrec.o \
						$(LB)/MSettingsImaging.o \
						$(LB)/MSettingsEventSelections.o \
						$(LB)/MBPData.o \
						$(LB)/MBPDataImage.o \
						$(LB)/MBPDataImageOneByte.o \
						$(LB)/MBPDataSparseImage.o \
						$(LB)/MBPDataSparseImageOneByte.o \
						$(LB)/MImager.o \
						$(LB)/MImagerExternallyManaged.o \
						$(LB)/MProjection.o \
						$(LB)/MBackprojection.o \
						$(LB)/MBackprojectionCartesian.o \
						$(LB)/MBackprojectionSphere.o \
						$(LB)/MEventSelector.o \
						$(LB)/MLMLAlgorithms.o \
						$(LB)/MLMLClassicEM.o \
						$(LB)/MLMLOSEM.o \
						$(LB)/MPointSource.o \
						$(LB)/MPointSourceList.o \
						$(LB)/MPointSourceSelector.o \
						$(LB)/MResponse.o \
						$(LB)/MResponseEnergyLeakage.o \
						$(LB)/MResponseGaussian.o \
						$(LB)/MResponseGaussianByUncertainties.o \
						$(LB)/MResponsePRM.o \
						$(LB)/MImageGalactic.o \
						$(LB)/MImageSpheric.o \
						$(LB)/MSensitivity.o \
						$(LB)/MEarthHorizon.o \
						$(LB)/MStandardAnalysis.o \


SGOBJ = $(LB)/libMimrecGui.$(DLL)\

SLOBJ = $(LB)/libMimrec.$(DLL)

#----------------------------------------------------------------
# Commands:
#

all: $(PRG)

lib: $(SLOBJ)

gui: $(SGOBJ)

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
		@rm -f $(MOBJS) $(GOBJS) $(LOBJS) $(ROBJS) $(SGOBJ) $(SLOBJ) $(RCINT)

#----------------------------------------------------------------
# Dependencies:
#


$(SLOBJ): $(LOBJS)
	@echo "Linking $(subst $(LB)/,,$@) ..."
ifeq ($(ARCH),macosx)
	$(LD) $(SOFLAGS) $(LOBJS) $(ROBJS) $(RCINT) $(GLIBS) $(LIBS) -o $(SLOBJ)
	$(LD) -bundle -undefined suppress -Wl,-x $(LDFLAGS) $^ $(GLIBS) $(LIBS) \
	   -o $(subst .$(DLL),.so,$@)
else
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(LOBJS) $(ROBJS) $(RCINT) $(GLIBS) $(LIBS) -o $(SLOBJ)
endif

$(SGOBJ): $(GOBJS) $(LOBJS)
	@echo "Linking $(subst $(LB)/,,$@) ..."
ifeq ($(ARCH),macosx)
	$(LD) $(SOFLAGS) $(GOBJS) $(LOBJS) $(ROBJS) $(RCINT) $(GLIBS) $(LIBS) -o $(SGOBJ)
	$(LD) -bundle -undefined suppress -Wl,-x $(LDFLAGS) $^ $(GLIBS) $(LIBS) \
	   -o $(subst .$(DLL),.so,$@)
else
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(GOBJS) $(LOBJS) $(ROBJS) $(RCINT) $(GLIBS) $(LIBS) -o $(SGOBJ)
endif

$(MOBJS): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(GOBJS): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(LOBJS): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@


$(PRG): $(SGOBJ) $(SLOBJ) $(MOBJS)
	@echo "Linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(MIMRECLIBS) $(GLIBS) $(LIBS) -o $(PRG)


#
#----------------------------------------------------------------



