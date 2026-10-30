#----------------------------------------------------------------
#
#  Makefile for MEGAlib - addon directory
#
#  Copyright (C) by the MEGAlib contributors.
#
#  This file is part of MEGAlib.
#
#  MEGAlib is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Lesser General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  MEGAlib is distributed in the hope that it will be useful, but WITHOUT
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
#  License (License.md) for more details.
#
#  SPDX-License-Identifier: LGPL-3.0-or-later
#
#
#  All *.cpp and *.cxx files represent exactly one program!
#
#----------------------------------------------------------------



#----------------------------------------------------------------
# Definitions:
#

PRGSX := \
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
  $(BN)/TraMerger \
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

FITS := \
	$(BN)/TraFitsConverter \
	$(BN)/ConvertMGGPOD \
	$(BN)/ResponseToXSPEC \
	
MPI := $(BN)/bundler



MLIBS := -L$(LB) -lMimrecGui -lMimrec -lRevanGui -lRevan -lSivanGui -lSivan -lSpectralyzeGui -lSpectralyze -lGeomegaGui -lGeomega -lCommonMisc -lCommonGui  

EVERYTHING := $(PRGSX) $(FITS)

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
	$(LD) $(LDFLAGS) $(CXXFLAGS) $(HEACFLAGS) $(DEFINES) $< $(MLIBS) $(GLIBS) $(LIBS) $(HEALIBS) -o $@
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



