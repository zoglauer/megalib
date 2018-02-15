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

<<<<<<< 2f715ab8a61be457c860bd99b888a7927da6763e
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

=======
OBJS      = $(LB)/MGlobal.o \
	$(LB)/MAssert.o \
	$(LB)/MCoordinateSystem.o \
	$(LB)/MExceptions.o \
	$(LB)/MAtmosphericAbsorption.o \
	$(LB)/MResponseMatrix.o \
	$(LB)/MResponseMatrixOx.o \
	$(LB)/MResponseMatrixO1.o \
	$(LB)/MResponseMatrixO2.o \
	$(LB)/MResponseMatrixO3.o \
	$(LB)/MResponseMatrixO4.o \
	$(LB)/MResponseMatrixO5.o \
	$(LB)/MResponseMatrixO6.o \
	$(LB)/MResponseMatrixO7.o \
	$(LB)/MResponseMatrixO8.o \
	$(LB)/MResponseMatrixO9.o \
	$(LB)/MResponseMatrixO10.o \
	$(LB)/MResponseMatrixO11.o \
	$(LB)/MResponseMatrixO12.o \
	$(LB)/MResponseMatrixO13.o \
	$(LB)/MResponseMatrixO14.o \
	$(LB)/MResponseMatrixO15.o \
	$(LB)/MResponseMatrixO16.o \
	$(LB)/MResponseMatrixO17.o \
	$(LB)/MResponseMatrixON.o \
	$(LB)/MResponseMatrixAxis.o \
	$(LB)/MResponseMatrixAxisSpheric.o \
	$(LB)/MString.o \
	$(LB)/MStreamBuffer.o \
	$(LB)/MStreams.o \
	$(LB)/MFitFunctions.o \
	$(LB)/MInterface.o \
	$(LB)/MCSVFunctionLoader.o \
	$(LB)/MFile.o \
	$(LB)/MFileEvents.o \
	$(LB)/MFileEventsRawa.o \
	$(LB)/MFileEventsTra.o \
	$(LB)/MFileManager.o \
	$(LB)/MFileResponse.o \
	$(LB)/MFileInfo.o \
	$(LB)/MFileHistograms.o \
	$(LB)/MImage.o \
	$(LB)/MImage2D.o \
	$(LB)/MImage3D.o \
	$(LB)/MImageContainer.o \
	$(LB)/MMath.o \
	$(LB)/MPairEvent.o \
	$(LB)/MSystem.o \
	$(LB)/MJulianDay.o \
	$(LB)/MTokenizer.o \
	$(LB)/MConnection.o \
	$(LB)/MTransceiverTcpIpBinary.o \
	$(LB)/MTransceiverTcpIp.o \
	$(LB)/MTransceiverUDP.o \
	$(LB)/MRotationInterface.o \
	$(LB)/MPhysicalEvent.o \
	$(LB)/MPhysicalEventHit.o \
	$(LB)/MComptonEvent.o \
	$(LB)/MMuonEvent.o \
	$(LB)/MPhotoEvent.o \
	$(LB)/MPETEvent.o \
	$(LB)/MMultiEvent.o \
	$(LB)/MUnidentifiableEvent.o \
	$(LB)/MParser.o \
	$(LB)/MFunction.o \
	$(LB)/MFunction2D.o \
	$(LB)/MFunction3D.o \
	$(LB)/MFunction3DSpherical.o \
	$(LB)/MVector.o \
	$(LB)/MRotation.o \
	$(LB)/MQuaternion.o \
	$(LB)/MPrelude.o \
	$(LB)/MGTI.o \
	$(LB)/MTime.o \
	$(LB)/MTimer.o \
	$(LB)/MIsotope.o \
	$(LB)/MIsotopeStore.o \
	$(LB)/MSettingsInterface.o \
	$(LB)/MSettings.o \
	$(LB)/MSettingsGlobal.o \
	$(LB)/MSettingsBasicFiles.o \
	$(LB)/MBinner.o \
	$(LB)/MBinnerFixedNumberOfBins.o \
	$(LB)/MBinnerFixedCountsPerBin.o \
	$(LB)/MBinnerBayesianBlocks.o \
	$(LB)/MBinnerFISBEL.o \
	$(LB)/MXmlData.o \
	$(LB)/MXmlNode.o \
	$(LB)/MXmlAttribute.o \
	$(LB)/MXmlDocument.o \


SOBJ = $(LB)/libCommonMisc.$(DLL)
>>>>>>> ADD: New event type classes

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
