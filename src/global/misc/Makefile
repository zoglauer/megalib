#----------------------------------------------------------------
#
#  Makefile for MEGAlib 
#
#  Author: Andreas Zoglauer
#
#----------------------------------------------------------------



#----------------------------------------------------------------
# Base definitions:
#

LIBRARY = CommonMisc

FILES = MGlobal \
	MAssert \
	MCoordinateSystem \
	MExceptions \
	MAtmosphericAbsorption \
	MResponseMatrix \
	MResponseMatrixOx \
	MResponseMatrixO1 \
	MResponseMatrixO2 \
	MResponseMatrixO3 \
	MResponseMatrixO4 \
	MResponseMatrixO5 \
	MResponseMatrixO6 \
	MResponseMatrixO7 \
	MResponseMatrixO8 \
	MResponseMatrixO9 \
	MResponseMatrixO10 \
	MResponseMatrixO11 \
	MResponseMatrixO12 \
	MResponseMatrixO13 \
	MResponseMatrixO14 \
	MResponseMatrixO15 \
	MResponseMatrixO16 \
	MResponseMatrixO17 \
	MResponseMatrixON \
	MResponseMatrixAxis \
	MResponseMatrixAxisSpheric \
	MString \
	MStreamBuffer \
	MStreams \
	MFitFunctions \
	MInterface \
	MCSVFunctionLoader \
	MFile \
	MFileEvents \
	MFileEventsRawa \
	MFileEventsTra \
	MFileManager \
	MFileResponse \
	MFileInfo \
	MFileHistograms \
	MImage \
	MImage2D \
	MImage3D \
	MImageContainer \
	MMath \
	MPairEvent \
	MSystem \
	MJulianDay \
	MTokenizer \
	MConnection \
	MTransceiverTcpIpBinary \
	MTransceiverTcpIp \
	MTransceiverUDP \
	MRotationInterface \
	MPhysicalEvent \
	MPhysicalEventHit \
	MComptonEvent \
	MMuonEvent \
	MPhotoEvent \
	MPETEvent \
	MMultiEvent \
	MUnidentifiableEvent \
	MParser \
	MFunction \
	MFunction2D \
	MFunction3D \
	MFunction3DSpherical \
	MVector \
	MRotation \
	MQuaternion \
	MPrelude \
	MGTI \
	MTime \
	MTimer \
	MIsotope \
	MIsotopeStore \
	MSettingsInterface \
	MSettings \
	MSettingsGlobal \
	MSettingsBasicFiles \
	MBinner \
	MBinnerFixedNumberOfBins \
	MBinnerFixedCountsPerBin \
	MBinnerBayesianBlocks \
	MBinnerFISBEL \
	MXmlData \
	MXmlNode \
	MXmlAttribute \
	MXmlDocument \
	MUnitTest \


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


#----------------------------------------------------------------
# Commands:
#

all: $(SHAREDLIB)

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

#----------------------------------------------------------------
# Dependencies:
#

$(DICTIONARY): $(HEADERS)
	@echo "Generating $(DICTIONARYNAME)_LinkDef ..."
	@$(BN)/generatelinkdef -o $(LINKDEF) -i $(HEADERS)
	@echo "Generating dictionary ..."
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

#
#----------------------------------------------------------------
