#----------------------------------------------------------------
#
# Makefile of the end-to-end testing framework: the library libEndToEnd, the programs testdrive and testdrivercalibration, and the unit tests
#
#
# The library holds the test base class, the report with the results and references, the reference campaign, and the test driver
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

LIBRARY := EndToEnd

FILES := MEndToEndRecord \
	MEndToEndResult \
	MEndToEndResultList \
	MEndToEndReference \
	MEndToEndReferenceList \
	MEndToEndFingerprint \
	MEndToEndReport \
	MEndToEndTest \
	MEndToEndCampaignAnalysis \
	MEndToEndCampaign \
	MTestDriver \

LIBRARIES := \
	-lSivan \
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

EXE_DRIVER := $(BN)/testdrive $(BN)/testdrivercalibration

CXX_UT := $(wildcard unittests/*.cxx)
EXE_UT := $(patsubst %.cxx,%,$(CXX_UT))
EXE_UT := $(patsubst unittests/%,$(BN)/%,$(EXE_UT))


#----------------------------------------------------------------
# Commands:
#

.PHONY: all tests lib link clean

all: $(EXE_DRIVER)

tests: $(EXE_DRIVER) $(EXE_UT)

lib: $(SHAREDLIB)

link:
	@$(LINK) $(shell pwd)/inc/*.h $(IN)

clean:
	@rm -f src/*~
	@rm -f src/\#*
	@rm -f inc/*~
	@rm -f inc/\#*
	@rm -f *~
	@rm -f \#*
	@rm -f $(OBJECTS) $(SHAREDLIB) $(DICTIONARY) $(DICTIONARYOBJECT) $(LINKDEF) $(ROOTMAP) $(LB)/$(ROOTPCM)
	@rm -f $(EXE_DRIVER)
	@rm -f $(EXE_UT)


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

$(BN)/testdrive: src/TestDriver.cxx $(SHAREDLIB)
	@echo "Compiling and linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(CXXFLAGS) $(LDFLAGS) $< $(SHAREDLIB) $(LIBRARIES) $(GLIBS) $(LIBS) -o $@

$(BN)/testdrivercalibration: src/TestDriverCalibration.cxx inc/TestDriverCalibration.h $(SHAREDLIB)
	@echo "Compiling and linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(CXXFLAGS) $(LDFLAGS) $< $(SHAREDLIB) $(LIBRARIES) $(GLIBS) $(LIBS) -o $@

$(EXE_UT): $(BN)/%: unittests/%.cxx $(SHAREDLIB)
	@echo "Compiling and linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(CXXFLAGS) $(LDFLAGS) $< $(SHAREDLIB) $(LIBRARIES) $(GLIBS) $(LIBS) -o $@

#
#----------------------------------------------------------------
