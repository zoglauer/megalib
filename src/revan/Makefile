#----------------------------------------------------------------
#
#  Makefile for Revan
#
#  Author: Andreas Zoglauer
#
#----------------------------------------------------------------



#----------------------------------------------------------------
# Base definitions:
#

<<<<<<< ec0e5895216066f58867cdf7a1156bd8853e590b
PROGRAM = revan

MAIN  = MRevanMain

LIBRARY = Revan

FILES = MGeometryRevan \
	MRawEventIncarnations \
	MRawEventAnalyzer \
	MFileEventsEvta \
	MFileDecay \
	MERConstruction \
	MERNoising \
	MERCoincidence \
	MERClusterize \
	MERTrack \
	MERTrackPearson \
	MERTrackRank \
	MERTrackBayesian \
	MERTrackGas \
	MERTrackDirectional \
	MERTrackChiSquare \
	MERCSR \
	MERCSRChiSquare \
	MERCSREnergyRecovery \
	MERCSRBayesian \
	MERCSRTMVA \
	MERCSRTMVAMethods \
	MERCSRToF \
	MERCSRToFWithEnergyRecovery \
	MERCSRDataSet \
	MERDecay \
	MSettingsRevan \
	MSettingsEventReconstruction \
	MRESE \
	MRESEList \
	MRESEIterator \
	MREHit \
	MRECluster \
	MRETrack \
	MRERawEvent \
	MREAM \
	MREAMDriftChamberEnergy \
	MREAMGuardRingHit \
	MREAMDirectional \
	MREAMStartInformation \
	MERQualityDataSet \
	MERStripPairingDataSet \
	
	
LIBRARY_UI = RevanGui

FILES_UI = \
	MInterfaceRevan \
	MGUIRevanMain \
	MGUIERAlgorithm \
	MGUIOptionsGeneral \
	MGUIOptionsCoincidence \
	MGUIOptionsClustering \
	MGUIOptionsTracking \
	MGUIOptionsCSR \
	MGUIOptionsDecay \
	MGUIExportSpectrum \
	MGUIReconstructedSpectrum \

LIBRARIES = \
	-lGeomega \
	-lSpectralyze \
	-lSpectralyzeGui \
	-lCommonMisc \
	-lCommonGui \

	
#----------------------------------------------------------------
# Derived definitions:
#

OBJECTS=$(patsubst %, $(LB)/%.o, $(FILES))
HEADERS=$(patsubst %, inc/%.h, $(FILES))
SHAREDLIB=$(LB)/lib$(LIBRARY).$(DLL)

DICTIONARYNAME=$(LIBRARY)_Dictionary
DICTIONARY=$(LB)/$(DICTIONARYNAME).cxx
DICTIONARYOBJECT=$(LB)/$(DICTIONARYNAME).o
LINKDEF=$(LB)/$(DICTIONARYNAME)_LinkDef.h
ROOTMAP=$(LB)/lib$(LIBRARY).rootmap
ROOTPCM=lib$(LIBRARY)_rdict.pcm


OBJECTS_UI=$(patsubst %, $(LB)/%.o, $(FILES_UI))
HEADERS_UI=$(patsubst %, inc/%.h, $(FILES_UI))
SHAREDLIB_UI=$(LB)/lib$(LIBRARY_UI).$(DLL)

DICTIONARYNAME_UI=$(LIBRARY_UI)_Dictionary
DICTIONARY_UI=$(LB)/$(DICTIONARYNAME_UI).cxx
DICTIONARYOBJECT_UI=$(LB)/$(DICTIONARYNAME_UI).o
LINKDEF_UI=$(LB)/$(DICTIONARYNAME_UI)_LinkDef.h
ROOTMAP_UI=$(LB)/lib$(LIBRARY_UI).rootmap
ROOTPCM_UI=lib$(LIBRARY_UI)_rdict.pcm


OBJECT_MAIN=$(patsubst %, $(LB)/%.o, $(MAIN))
EXECUTABLE=$(BN)/$(PROGRAM)

=======
PRG = 	$(BN)/revan

MOBJS = $(LB)/MRevanMain.o

GOBJS = $(LB)/MInterfaceRevan.o \
	$(LB)/MGUIRevanMain.o \
	$(LB)/MGUIERAlgorithm.o \
	$(LB)/MGUIOptionsGeneral.o \
	$(LB)/MGUIOptionsCoincidence.o \
	$(LB)/MGUIOptionsHitClustering.o \
	$(LB)/MGUIOptionsTracking.o \
	$(LB)/MGUIOptionsCSR.o \
	$(LB)/MGUIOptionsDecay.o \
	$(LB)/MGUIExportSpectrum.o \
	$(LB)/MGUIReconstructedSpectrum.o \

LOBJS = $(LB)/MGeometryRevan.o \
	$(LB)/MRawEventIncarnations.o \
	$(LB)/MRawEventAnalyzer.o \
	$(LB)/MFileEventsEvta.o \
	$(LB)/MFileDecay.o \
	$(LB)/MERConstruction.o \
	$(LB)/MERNoising.o \
	$(LB)/MERCoincidence.o \
	$(LB)/MERHitClusterizer.o \
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
	$(LB)/MERStripPairingDataSet.o \
				

OBJS = $(MOBJS) $(LOBJS) $(GOBJS)

SGOBJ = $(LB)/libRevanGui.$(DLL)
SLOBJ = $(LB)/libRevan.$(DLL)
>>>>>>> RFC: Refactoring MRawEventList --> MRawEventIncarnations, to make it clear that the list just contains different incarnation sof the same event

#----------------------------------------------------------------
# Commands:
#

all: $(EXECUTABLE)

lib: $(SHAREDLIB) 

link:
	@$(LINK) $(shell pwd)/inc/*.h $(IN)
	@echo "$(OBJECTS) $(DICTIONARYOBJECT) $(OBJECTS_UI) $(DICTIONARYOBJECT_UI)" >> $(LB)/AllObjects.txt

clean:
	@rm -f src/*~
	@rm -f src/\#*
	@rm -f inc/*~
	@rm -f inc/\#*
	@rm -f *~
	@rm -f \#*
	@rm -f $(OBJECTS) $(SHAREDLIB) $(DICTIONARY) $(DICTIONARYOBJECT) $(LINKDEF) $(ROOTMAP) $(LB)/$(ROOTPCM)
	@rm -f $(OBJECTS_UI) $(SHAREDLIB_UI) $(DICTIONARY_UI) $(DICTIONARYOBJECT_UI) $(LINKDEF_UI) $(ROOTMAP_UI) $(LB)/$(ROOTPCM_UI)
	@rm -f $(OBJECTS_MAIN) $(EXECUTABLE)

#----------------------------------------------------------------
# Dependencies:
#

$(DICTIONARY): $(HEADERS)
	@echo "Generating $(DICTIONARYNAME)_LinkDef.h ..."
	@$(BN)/generatelinkdef -o $(LINKDEF) -i $(HEADERS)
	@echo "Generating dictionary $(DICTIONARYNAME).cxx..."
	@rootcling -f $@ -I$(IN) -D___CLING___ -rmf $(ROOTMAP) -s lib$(LIBRARY) -c $^ $(LINKDEF)
	@mv $(ROOTPCM) $(LB)

$(SHAREDLIB): $(OBJECTS) $(DICTIONARYOBJECT)
	@echo "Linking lib$(LIBRARY) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(DICTIONARYOBJECT) $(OBJECTS) $(GLIBS) $(LIBS) -o $(SHAREDLIB)

$(OBJECTS): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(DICTIONARYOBJECT): $(DICTIONARY)
	@echo "Compiling dictionary ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@



$(DICTIONARY_UI): $(HEADERS_UI)
	@echo "Generating $(DICTIONARYNAME_UI)_LinkDef.h ..."
	@$(BN)/generatelinkdef -o $(LINKDEF_UI) -i $(HEADERS_UI)
	@echo "Generating dictionary $(DICTIONARYNAME_UI).cxx..."
	@rootcling -f $@ -I$(IN) -D___CLING___ -rmf $(ROOTMAP_UI) -s lib$(LIBRARY_UI) -c $^ $(LINKDEF_UI)
	@mv $(ROOTPCM_UI) $(LB)

$(SHAREDLIB_UI): $(OBJECTS_UI) $(DICTIONARYOBJECT_UI)
	@echo "Linking lib$(LIBRARY_UI) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(DICTIONARYOBJECT_UI) $(OBJECTS_UI) $(GLIBS) $(LIBS) -o $(SHAREDLIB_UI)

$(OBJECTS_UI): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(DICTIONARYOBJECT_UI): $(DICTIONARY_UI)
	@echo "Compiling dictionary ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@



$(OBJECT_MAIN): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(EXECUTABLE): $(SHAREDLIB_UI) $(SHAREDLIB) $(OBJECT_MAIN)
	@echo "Linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(OBJECT_MAIN) $(SHAREDLIB_UI) $(SHAREDLIB) $(LIBRARIES) $(GLIBS) $(LIBS) -o $(EXECUTABLE)

	
#
#----------------------------------------------------------------



