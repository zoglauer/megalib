#----------------------------------------------------------------
#
#  Makefile for MEGAlib - addon directory
#
#  Author: Andreas Zoglauer
#
#
#  All *.cpp and *.cxx files represent exactly one program!
#
#----------------------------------------------------------------



#----------------------------------------------------------------
# Definitions:
#

PRGSX = \
	$(BN)/SensitivityOptimizer \
	$(BN)/Mask \
	$(BN)/ConvertMGeant \
	$(BN)/ConvertACTtools \
	$(BN)/SimAnalyzer \
	$(BN)/SimRewriter \
	$(BN)/SimCombiner \
	$(BN)/SimBinaryConverter \
	$(BN)/SimRandomCoincidence \
	$(BN)/TraAnalyzer \
	$(BN)/DecayAnalyzer \
	$(BN)/BackgroundMixer \
	$(BN)/SimpleComptonImaging \
	$(BN)/ExternalDataAnalysisExample \
	$(BN)/ShowHistograms \
	$(BN)/CompareHistograms \
	$(BN)/DistanceOptimizerForEventClusterizer \
	$(BN)/Revoxelizer \
	$(BN)/IsotopeFileSplitter \
	$(BN)/VariableSourceDetector \

FITS = \
	$(BN)/TraFitsConverter \
	$(BN)/ConvertMGGPOD \

MPI = $(BN)/bundler



MLIBS = -L$(LB) -lMimrecGui -lMimrec -lRevanGui -lRevan -lSivanGui -lSivan -lSpectralyzeGui -lSpectralyze -lGeomegaGui -lGeomega -lCommonMisc -lCommonGui  

EVERYTHING = $(PRGSX) $(FITS)

#----------------------------------------------------------------
# Commands:
#

all: $(EVERYTHING) 

clean:
	@rm -f *~
	@rm -f \#*
	@rm -f *.o
	@rm -f $(PRGSX)
	@rm -f $(PRGSN)
	@rm -f $(FITS)
	@rm -f $(MPI)


#----------------------------------------------------------------
# Dependencies:
#

#$(PRGSP): $(BN)/%: %.cpp
#ifneq "$(shell cat $(TOP)/config/Makefile.EmptyDir )" "$(shell cat $(TOP)/src/megalyze/Makefile )"
#	@echo "Compiling and linking $(subst $(BN)/,,$@) ..."
#	$(LD) $(LDFLAGS) $(CXXFLAGS) $(ALLLIBS) $(DEFINES) $< $(GLIBS) $(LIBS) -o $@
#endif

$(FITS): $(BN)/%: %.cxx
ifeq "$(strip $(HEASOFTINSTALLED))" "0"
	@echo "Compiling and linking $(subst $(BN)/,,$@) ..."
	$(LD) $(LDFLAGS) $(CXXFLAGS) $(DEFINES) -I$(LHEASOFT)/include $< $(MLIBS) -lcfitsio -lm $(GLIBS) $(LIBS) -L$(LHEASOFT)/lib -o $@
endif

$(MPI): $(BN)/%: %.cxx
ifeq "$(strip $(MPIINSTALLED))" "0"
	@echo "Compiling and linking $(subst $(BN)/,,$@) ..."
	mpic++ $(LDFLAGS) $(CXXFLAGS) $(DEFINES) $< $(MLIBS) $(GLIBS) $(LIBS) -o $@
endif

$(PRGSX): $(BN)/%: %.cxx
ifeq ($(wildcard $(SR)/mimrec/src), $(SR)/mimrec/src)
	@echo "Compiling and linking $(subst $(BN)/,,$@) ..."
	$(LD) $(LDFLAGS) $(CXXFLAGS) $(DEFINES) $< $(MLIBS) $(GLIBS) $(LIBS) -o $@
endif

$(PRGSN): $(BN)/%: %.cxx
ifneq ($(strip $(NuSTARExists)),)
	@echo "Compiling and linking $(subst $(BN)/,,$@) ..."
	$(LD) $(LDFLAGS) $(CXXFLAGS) $(DEFINES) $< $(MLIBS) $(GLIBS) $(LIBS) -o $@
endif

#
#----------------------------------------------------------------



