#----------------------------------------------------------------
#
#  Makefile for MEGAlib / cosima
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
#----------------------------------------------------------------


#----------------------------------------------------------------
# Definitions:
#

PRG        := $(BN)/cosima
CPP_MAIN   := src/MCCosima.cc
CPP_FILES  := $(wildcard src/*.cc)
CPP_FILES  := $(filter-out $(CPP_MAIN),$(CPP_FILES))
OBJ_FILES  := $(addprefix $(LB)/,$(notdir $(CPP_FILES:.cc=.o)))
SOBJ       := $(LB)/libCosima.$(DLL)
G4_CFLAGS  := $(shell geant4-config --cflags) -DG4VIS_USE -Wno-deprecated-copy -Wno-overloaded-virtual
G4_CFLAGS  := $(filter-out -Wshadow, $(G4_CFLAGS))
G4_CFLAGS  := $(filter-out -Woverloaded-virtual, $(G4_CFLAGS))
G4_LIBS    := $(shell geant4-config --libs) -Wl,-rpath,$(shell geant4-config --prefix)/lib
LIBRARIES  := -lSivan -lRevan -lSpectralyze -lGeomega -lCommonMisc -lCommonGui
CPP_UT     := $(wildcard unittests/*.cc)
CPP_UT     := $(filter-out unittests/MCTestDummy.cc,$(CPP_UT))
PRG_UT     := $(patsubst %.cc,%,$(CPP_UT)) 
PRG_UT     := $(patsubst unittests/%,$(BN)/%,$(PRG_UT)) 

#----------------------------------------------------------------
# Commands:
#

all: $(PRG) $(PRG_UT)

lib: $(SOBJ) 

link:
	@$(LINK) $(shell pwd)/inc/*.hh $(IN)

clean:
	@rm -f src/*~
	@rm -f src/\#*
	@rm -f inc/*~
	@rm -f inc/\#*
	@rm -f *~
	@rm -f \#*
	@rm -f $(PRG)
	@rm -f $(OBJ_FILES) $(SOBJ)


#----------------------------------------------------------------
# Dependencies:
#

$(PRG): $(CPP_MAIN) $(SOBJ)
	@echo "Compiling and linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(CXXFLAGS) $(G4_CFLAGS) $(LDFLAGS) $(CPP_MAIN) $(SOBJ) $(LIBRARIES) $(GLIBS) $(LIBS) $(G4_LIBS) -o $(PRG)

$(SOBJ): $(OBJ_FILES)
	@echo "Linking $(subst $(LB)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(OBJ_FILES) $(G4_LIBS) $(GLIBS) $(LIBS) -o $(SOBJ)

$(LB)/%.o: src/%.cc inc/%.hh
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) $(G4_CFLAGS) -c -o $@ $<

$(PRG_UT): $(BN)/%: unittests/%.cc $(SOBJ)
	@echo "Compiling and linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(CXXFLAGS) $(G4_CFLAGS) $(LDFLAGS) $< $(SOBJ) $(LIBRARIES) $(GLIBS) $(LIBS) $(G4_LIBS) -o $@

#
#----------------------------------------------------------------
