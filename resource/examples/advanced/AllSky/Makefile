#-------------------------------------------------------------------------------
#
# Makefile for a stand alone application based on MEGAlib 
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
#
# Usage:
# make -f Makefile.StandAlone PRG=<filename without *.cxx>
#
# Options:
# CMD=<command line parameters>
#
# debug: call gdb
# clean: remove program and library
#
# Example:
# make -f Makefile.StandAlone PRG=Extract
#
# The Makefile needs a completely compiled MEGAlib to function properly.
# The program is automatically placed in $(MEGALIB)/bin.
#
#-------------------------------------------------------------------------------



#-------------------------------------------------------------------------------
# Program and directories

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

MPRG=Converter

MLIBS = -L$(LB) -lMimrecGui -lMimrec -lRevanGui -lRevan -lSivanGui -lSivan -lSpectralyzeGui -lSpectralyze -lGeomegaGui -lGeomega -lCommonMisc -lCommonGui  


#-------------------------------------------------------------------------------
# Included options (result of ./configure)

include $(TOP)/config/Makefile.options
include $(TOP)/config/Makefile.config

CXXFLAGS += -I$(IN)


#-------------------------------------------------------------------------------
# Commands:


all: $(BN)/$(MPRG)
ifneq ($(CMD),)
	@$(MPRG) $(CMD)
endif

only: $(BN)/$(MPRG)

run: $(BN)/$(MPRG)
	@$(MPRG) $(CMD)

debug:  $(BN)/$(MPRG)
	@echo "r $(CMD)" > /tmp/$(MPRG).ini
	@gdb $(BN)/$(MPRG) -x /tmp/$(MPRG).ini
	@rm -f /tmp/$(MPRG).ini

clean:
	rm -f $(LB)/$(MPRG).o
	rm -f $(BN)/$(MPRG)


#-------------------------------------------------------------------------------
# Dependencies:

$(BN)/$(MPRG): $(LB)/$(MPRG).o
	@$(LD) $(LDFLAGS) -I$(LHEASOFT)/include $< $(MLIBS) -lcfitsio -lm $(GLIBS) $(LIBS) -lHistPainter -L$(LHEASOFT)/lib -o $@
	@echo "Program $(MPRG) successfully linked... and starting..."


$(LB)/$(MPRG).o: $(MPRG).cxx
	@$(CXX) $(CXXFLAGS) -I$(LHEASOFT)/include -c $< -o $@
	@echo "Program $(MPRG) successfully compiled..."

#
#-------------------------------------------------------------------------------


