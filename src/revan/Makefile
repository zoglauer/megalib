#----------------------------------------------------------------
#
#  Makefile for Revan
#
#  Author: Andreas Zoglauer
#
#----------------------------------------------------------------



#----------------------------------------------------------------
# Definitions:
#

PRG = 	$(BN)/revan

MOBJS = $(LB)/MRevanMain.o

GOBJS = $(LB)/MInterfaceRevan.o \
	$(LB)/MGUIRevanMain.o \
	$(LB)/MGUIERAlgorithm.o \
	$(LB)/MGUIOptionsGeneral.o \
	$(LB)/MGUIOptionsCoincidence.o \
	$(LB)/MGUIOptionsClustering.o \
	$(LB)/MGUIOptionsTracking.o \
	$(LB)/MGUIOptionsCSR.o \
	$(LB)/MGUIOptionsDecay.o \
	$(LB)/MGUIExportSpectrum.o \
	$(LB)/MGUIReconstructedSpectrum.o \

LOBJS = $(LB)/MGeometryRevan.o \
	$(LB)/MRawEventList.o \
	$(LB)/MRawEventAnalyzer.o \
	$(LB)/MFileEventsEvta.o \
	$(LB)/MFileDecay.o \
	$(LB)/MERConstruction.o \
	$(LB)/MERNoising.o \
	$(LB)/MERCoincidence.o \
	$(LB)/MERClusterize.o \
	$(LB)/MERTrack.o \
	$(LB)/MERTrackPearson.o \
	$(LB)/MERTrackRank.o \
	$(LB)/MERTrackBayesian.o \
	$(LB)/MERTrackGas.o \
	$(LB)/MERTrackDirectional.o \
	$(LB)/MERTrackChiSquare.o \
	$(LB)/MERCSR.o \
	$(LB)/MERCSRChiSquare.o \
	$(LB)/MERCSREnergyRecovery.o \
	$(LB)/MERCSRBayesian.o \
	$(LB)/MERCSRTMVA.o \
	$(LB)/MERCSRTMVAMethods.o \
	$(LB)/MERCSRToF.o \
	$(LB)/MERCSRToFWithEnergyRecovery.o \
	$(LB)/MERCSRDataSet.o \
	$(LB)/MERDecay.o \
	$(LB)/MSettingsRevan.o \
	$(LB)/MSettingsEventReconstruction.o \
	$(LB)/MRESE.o \
	$(LB)/MRESEList.o \
	$(LB)/MRESEIterator.o \
	$(LB)/MREHit.o \
	$(LB)/MRECluster.o \
	$(LB)/MRETrack.o \
	$(LB)/MRERawEvent.o \
	$(LB)/MREAM.o \
	$(LB)/MREAMDriftChamberEnergy.o \
	$(LB)/MREAMGuardRingHit.o \
	$(LB)/MREAMDirectional.o \
	$(LB)/MREAMStartInformation.o \
	$(LB)/MERQualityDataSet.o \
				

OBJS = $(MOBJS) $(LOBJS) $(GOBJS)

SGOBJ = $(LB)/libRevanGui.$(DLL)
SLOBJ = $(LB)/libRevan.$(DLL)

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
		@rm -f $(BN)/revan
		@rm -f $(MOBJS) $(GOBJS) $(LOBJS) $(SGOBJ) $(SLOBJ)


#----------------------------------------------------------------
# Dependencies:
#

$(SLOBJ): $(LOBJS)
	@echo "Linking $(subst $(LB)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(LOBJS) $(GLIBS) $(LIBS) -o $(SLOBJ)

$(SGOBJ): $(GOBJS) $(LOBJS)
	@echo "Linking $(subst $(LB)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(GOBJS) $(LOBJS) $(GLIBS) $(LIBS) -o $(SGOBJ)

$(OBJS): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(PRG): $(MOBJS) $(SLOBJ) $(SGOBJ)
	@echo "Linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(REVANLIBS)  $(GLIBS) $(LIBS) -o $(PRG)

#
#----------------------------------------------------------------



