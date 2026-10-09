#----------------------------------------------------------------
#
# Makefile of the end-to-end tests
#
# Every ET*.cxx in this directory is a program which is placed in bin/
# The tests need the programs cosima, revan, and mimrec at run time (they fail, not skip, without them), the build only needs the common libraries
#
# Copyright (C) by the MEGAlib contributors.
#
# This file is part of MEGAlib.
#
# MEGAlib is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# MEGAlib is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
# License (License.md) for more details.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
#
#----------------------------------------------------------------


#----------------------------------------------------------------
# Definitions:
#

LIBRARIES := -lSivan -lGeomega -lCommonMisc -lCommonGui
CXX_ET := $(wildcard src/ET*.cxx)
EXE_ET := $(patsubst src/%.cxx,$(BN)/%,$(CXX_ET))
# The test driver and its calibration: the programs are named in lower case like the main MEGAlib programs, the source files are named after their class
EXE_DRIVER := $(BN)/testdrive $(BN)/testdrivercalibration
CXX_UT := $(wildcard unittests/UT*.cxx)
EXE_UT := $(patsubst unittests/%.cxx,$(BN)/%,$(CXX_UT))
EXE_UT_DRIVER := $(BN)/UTTestDriver
EXE_UT_ET := $(filter-out $(EXE_UT_DRIVER),$(EXE_UT))


#----------------------------------------------------------------
# Commands:
#

all: $(EXE_ET) $(EXE_DRIVER)

tests: $(EXE_ET) $(EXE_DRIVER) $(EXE_UT)

lib:

link:
	@$(LINK) $(shell pwd)/inc/*.h $(IN)

clean:
	@rm -f $(EXE_ET) $(EXE_DRIVER) $(EXE_UT)


#----------------------------------------------------------------
# Dependencies:
#

$(EXE_ET): $(BN)/%: src/%.cxx inc/%.h src/MEndToEndTest.cxx inc/MEndToEndTest.h $(LB)/libSivan.$(DLL)
	@echo "Compiling and linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(CXXFLAGS) $(LDFLAGS) $< src/MEndToEndTest.cxx $(LIBRARIES) $(GLIBS) $(LIBS) -o $@

$(BN)/testdrive: src/TestDriver.cxx src/MTestDriver.cxx inc/MTestDriver.h $(LB)/libCommonMisc.$(DLL)
	@echo "Compiling and linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(CXXFLAGS) $(LDFLAGS) src/TestDriver.cxx src/MTestDriver.cxx $(LIBRARIES) $(GLIBS) $(LIBS) -o $@

$(BN)/testdrivercalibration: src/TestDriverCalibration.cxx inc/TestDriverCalibration.h src/MEndToEndTest.cxx inc/MEndToEndTest.h $(LB)/libSivan.$(DLL)
	@echo "Compiling and linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(CXXFLAGS) $(LDFLAGS) src/TestDriverCalibration.cxx src/MEndToEndTest.cxx $(LIBRARIES) $(GLIBS) $(LIBS) -o $@

$(EXE_UT_DRIVER): $(BN)/%: unittests/%.cxx src/MTestDriver.cxx inc/MTestDriver.h $(LB)/libCommonMisc.$(DLL)
	@echo "Compiling and linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(CXXFLAGS) $(LDFLAGS) $< src/MTestDriver.cxx $(LIBRARIES) $(GLIBS) $(LIBS) -o $@

$(EXE_UT_ET): $(BN)/%: unittests/%.cxx src/MEndToEndTest.cxx inc/MEndToEndTest.h $(LB)/libSivan.$(DLL)
	@echo "Compiling and linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(CXXFLAGS) $(LDFLAGS) $< src/MEndToEndTest.cxx $(LIBRARIES) $(GLIBS) $(LIBS) -o $@

#
#----------------------------------------------------------------
