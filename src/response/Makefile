#----------------------------------------------------------------
#
#  Makefile for Response
#
#  Author: Andreas Zoglauer
#
#----------------------------------------------------------------



#----------------------------------------------------------------
# Base definitions:
#


PROGRAM1 := responsemanipulator
MAIN1    := MResponseManipulatorMain

PROGRAM2 := responsecreator
MAIN2    := MResponseCreatorMain

LIBRARY := ResponseCreator

FILES := MResponseCreator \
	MResponseManipulator \
	MResponseBuilder \
	MResponseBase \
	MResponseEventClusterizerTMVAEventFile \
	MResponseEventClusterizerTMVA \
	MResponseClusteringDSS \
	MResponseTracking \
	MResponseTrackWithinCrossStripDetectorTMVAEventFile \
	MResponseMultipleCompton \
	MResponseMultipleComptonBayes \
	MResponseMultipleComptonEventFile \
	MResponseMultipleComptonLens \
	MResponseMultipleComptonTMVA \
	MResponseFirstInteractionPosition \
	MResponseSpectral \
	MResponseImaging \
	MResponseImagingARM \
	MResponseImagingEfficiency \
	MResponseImagingEfficiencyNearField \
	MResponseImagingBinnedMode \
	MResponseImagingCodedMask \
	MResponsePolarizationBinnedMode \
	MResponseEarthHorizon \
	MResponseEventQuality \
	MResponseEventQualityTMVAEventFile \
	MResponseStripPairingTMVAEventFile \
	MResponseComptelDataSpace \
	
LIBRARIES := \
	-lRevan \
	-lSivan \
	-lMimrec \
	-lSpectralyze \
	-lGeomega \
	-lCommonMisc \
	-lCommonGui \


#----------------------------------------------------------------
# Derived definitions:
#

OBJECTS:=$(patsubst %, $(LB)/%.o, $(FILES))
HEADERS:=$(patsubst %, inc/%.h, $(FILES))
SHAREDLIB:=$(LB)/lib$(LIBRARY).$(DLL)

DICTIONARYNAME:=$(LIBRARY)_Dictionary
DICTIONARY:=$(LB)/$(DICTIONARYNAME).cxx
DICTIONARYOBJECT:=$(LB)/$(DICTIONARYNAME).o
LINKDEF:=$(LB)/$(DICTIONARYNAME)_LinkDef.h
ROOTMAP:=$(LB)/lib$(LIBRARY).rootmap
ROOTPCM:=lib$(LIBRARY)_rdict.pcm

OBJECT_MAIN1:=$(patsubst %, $(LB)/%.o, $(MAIN1))
EXECUTABLE1:=$(BN)/$(PROGRAM1)

OBJECT_MAIN2:=$(patsubst %, $(LB)/%.o, $(MAIN2))
EXECUTABLE2:=$(BN)/$(PROGRAM2)


#----------------------------------------------------------------
# Commands:
#

all: $(EXECUTABLE1) $(EXECUTABLE2)

lib: $(SHAREDLIB) 

link:
	@$(LINK) $(shell pwd)/inc/*.h $(IN)
	@echo "$(OBJECTS) $(DICTIONARYOBJECT)" >> $(LB)/AllObjects.txt

clean:
	@rm -f src/*~
	@rm -f src/\#*
	@rm -f inc/*~
	@rm -f inc/\#*
	@rm -f *~
	@rm -f \#*
	@rm -f $(OBJECTS) $(SHAREDLIB) $(DICTIONARY) $(DICTIONARYOBJECT) $(LINKDEF) $(ROOTMAP) $(LB)/$(ROOTPCM)
	@rm -f $(OBJECTS_MAIN1) $(EXECUTABLE1)
	@rm -f $(OBJECTS_MAIN2) $(EXECUTABLE2)

	
#----------------------------------------------------------------
# Dependencies:
#

$(DICTIONARY): $(HEADERS)
	@echo "Generating $(DICTIONARYNAME)_LinkDef.h ..."
	@$(BN)/generatelinkdef -o $(LINKDEF) -i $(HEADERS)
	@echo "Generating dictionary $(DICTIONARYNAME).cxx..."
	@rootcling -f $@ -I$(IN) -D___CLING___ -rmf $(ROOTMAP) -s lib$(LIBRARY) -c $^ $(LINKDEF)
	@mv $(ROOTPCM) $(LB)

$(SHAREDLIB): $(DICTIONARYOBJECT) $(OBJECTS)
	@echo "Linking lib$(LIBRARY) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(DICTIONARYOBJECT) $(OBJECTS) $(GLIBS) $(LIBS) -o $(SHAREDLIB)

$(OBJECTS): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(DICTIONARYOBJECT): $(DICTIONARY)
	@echo "Compiling dictionary ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@



$(OBJECT_MAIN1): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(EXECUTABLE1): $(SHAREDLIB) $(OBJECT_MAIN1)
	@echo "Linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(OBJECT_MAIN1) $(SHAREDLIB) $(LIBRARIES) $(GLIBS) $(LIBS) -o $(EXECUTABLE1)


$(OBJECT_MAIN2): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(EXECUTABLE2): $(SHAREDLIB) $(OBJECT_MAIN2)
	@echo "Linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(OBJECT_MAIN2) $(SHAREDLIB) $(LIBRARIES) $(GLIBS) $(LIBS) -o $(EXECUTABLE2)

	
#
#----------------------------------------------------------------
