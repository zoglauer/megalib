#----------------------------------------------------------------
#
#  Makefile for MEGAlib / fretalon / base
#
#  Author: Andreas Zoglauer
#
#----------------------------------------------------------------



#----------------------------------------------------------------
# Definitions:
#

LOBJS= \
$(LB)/MFretalonRegistry.o \
$(LB)/MFileReadOuts.o \
$(LB)/MReadOut.o \
$(LB)/MReadOutElement.o \
$(LB)/MReadOutElementStrip.o \
$(LB)/MReadOutElementDoubleStrip.o \
$(LB)/MReadOutData.o \
$(LB)/MReadOutDataInterfaceADCValue.o \
$(LB)/MReadOutDataInterfaceTiming.o \
$(LB)/MReadOutDataADCValue.o \
$(LB)/MReadOutDataADCValueWithTiming.o \
$(LB)/MReadOutSequence.o \
$(LB)/MReadOutDataGroup.o \
$(LB)/MReadOutCollection.o \
$(LB)/MReadOutStore.o \
$(LB)/MCalibrate.o \
$(LB)/MCalibrateEnergy.o \
$(LB)/MCalibrateEnergyAssignEnergies.o \
$(LB)/MCalibrateEnergyDetermineModel.o \
$(LB)/MCalibrateEnergyFindLines.o \
$(LB)/MCalibration.o \
$(LB)/MCalibrationSpectrum.o \
$(LB)/MCalibrationSpectralPoint.o \
$(LB)/MCalibrationStore.o \
$(LB)/MCalibrationFit.o \
$(LB)/MCalibrationFitGaussian.o \
$(LB)/MCalibrationFitGaussLandau.o \
$(LB)/MCalibrationModel.o \

SLOBJ = $(LB)/libFretalonBase.$(DLL)

#----------------------------------------------------------------
# Commands:
#

all: $(SLOBJ)

lib: $(SLOBJ) 

link:
	@$(LINK) $(shell pwd)/inc/*.h $(IN)

clean:
	@rm -f src/*~
	@rm -f src/\#*
	@rm -f inc/*~
	@rm -f inc/\#*
	@rm -f *~
	@rm -f \#*
	@rm -f $(LOBJS) $(SLOBJ)


#----------------------------------------------------------------
# Dependencies:
#

$(SLOBJ): $(LOBJS)
	@echo "Linking $(subst $(LB)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(LOBJS) $(GLIBS) $(LIBS) -o $(SLOBJ)

$(LOBJS): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

#
#----------------------------------------------------------------



