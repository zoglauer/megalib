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

SHELL=/bin/sh
CMD=

# Basic directories
TOP = $(MEGALIB)
SR  = $(TOP)/src
IN  = $(TOP)/include
LB  = $(TOP)/lib
BN  = $(TOP)/bin
HT  = $(TOP)/doc/html
CT  = $(TOP)/cint

PRG1       := modifiedcosima
PRG1_MAIN  := ModifiedCosima.cxx
PRG2       := quickparse
PRG2_MAIN  := QuickParse.cxx

G4_CFLAGS  := $(shell geant4-config --cflags)
G4_LIBS    := $(shell geant4-config --libs)

MLIBS      := -L$(LB) -lCosima -lMimrecGui -lMimrec -lRevanGui -lRevan -lSivanGui -lSivan -lSpectralyzeGui -lSpectralyze -lGeomegaGui -lGeomega -lCommonMisc -lCommonGui  

include $(TOP)/config/Makefile.options
include $(TOP)/config/Makefile.config

CXXFLAGS += -I$(IN)


#----------------------------------------------------------------
# Commands:
#

all: $(PRG1) $(PRG2)


#----------------------------------------------------------------
# Dependencies:
#

$(PRG1) : $(PRG1_MAIN)
	@echo "Compiling and linking $(PRG1) ..."
	@$(LD) $(CXXFLAGS) $(G4_CFLAGS) $(LDFLAGS) $(PRG1_MAIN) $(MLIBS) $(GLIBS) $(LIBS) $(G4_LIBS) -o $(BN)/$(PRG1)

$(PRG2): $(PRG2_MAIN)
	@echo "Compiling and linking $(PRG2) ..."
	@$(LD) -std=c++11 $(PRG2_MAIN) -o $(BN)/$(PRG2)


#
#----------------------------------------------------------------



