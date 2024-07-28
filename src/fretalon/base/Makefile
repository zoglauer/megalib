#----------------------------------------------------------------
#
#  Makefile for MEGAlib / fretalon / base
#
#  Author: Andreas Zoglauer
#
#----------------------------------------------------------------



#----------------------------------------------------------------
# Base definitions:
#

LIBRARY := FretalonBase

FILES := MFretalonRegistry \
	MFileReadOuts \
	MReadOut \
	MReadOutElement \
	MReadOutElementStrip \
	MReadOutElementDoubleStrip \
	MReadOutData \
	MReadOutDataADCValue \
	MReadOutDataTiming \
	MReadOutDataTemperature \
	MReadOutDataOrigins \
	MReadOutSequence \
	MReadOutDataGroup \
	MReadOutCollection \
	MReadOutStore \
	MCalibrate \
	MCalibrateEnergy \
	MCalibrateEnergyAssignEnergies \
	MCalibrateEnergyDetermineModel \
	MCalibrateEnergyFindLines \
	MCalibration \
	MCalibrationSpectrum \
	MCalibrationSpectralPoint \
	MCalibrationStore \
	MCalibrationFit \
	MCalibrationFitGaussian \
	MCalibrationFitGaussLandau \
	MCalibrationModel \

SLOBJ := libFretalonBase.$(DLL)

LIBRARIES := \
	-lSivan \
	-lGeomega \
	-lCommonGui \
	-lCommonMisc \

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


#----------------------------------------------------------------
# Commands:
#

all: $(SHAREDLIB)

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

#----------------------------------------------------------------
# Dependencies:
#

$(DICTIONARY): $(HEADERS)
	@echo "Generating $(DICTIONARYNAME)_LinkDef ..."
	@$(BN)/generatelinkdef -o $(LINKDEF) -i $(HEADERS)
	@echo "Generating dictionary ..."
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

#
#----------------------------------------------------------------
