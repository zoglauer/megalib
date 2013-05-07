#-------------------------------------------------------------------------------
#
# Makefile for a stand alone application based on MEGAlib 
#
# Copyright (C) 1998-2003 by Andreas Zoglauer.
# All rights reserved.
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
# make -f Makefile.StandAlone PRGPATH=src/addon PRG=Extract
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

MPRG=$(subst .cxx,,$(PRG))

MLIBS = -L$(LB) -lMimrecGui -lMimrec -lRevanGui -lRevan -lSivanGui -lSivan -lSpectralyzeGui -lSpectralyze -lGeomegaGui -lGeomega -lCommonMisc -lCommonGui  

#-------------------------------------------------------------------------------
# Included options (result of ./configure)

include $(TOP)/config/Makefile.options
include $(TOP)/config/Makefile.config

CXXFLAGS		+= -I$(IN)


#-------------------------------------------------------------------------------
# Commands:


all: $(BN)/$(MPRG)
				@$(MPRG) $(CMD)

only: $(BN)/$(MPRG)

debug:  $(BN)/$(MPRG)
				@echo "r $(CMD)" > /tmp/$(MPRG).ini
				@gdb $(BN)/$(MPRG) -x /tmp/$(MPRG).ini
				@rm -f /tmp/$(MPRG).ini

profile:  $(BN)/$(MPRG)
	@$(LD) $(LDFLAGS) $(LB)/*.o $(GLIBS) $(LIBS) -o $(BN)/$(MPRG)
	@$(BN)/$(MPRG)
	@gprof $(BN)/$(MPRG) gmon.out > gprof.out

clean:
				rm -f $(LB)/$(MPRG).o
				rm -f $(BN)/$(MPRG)


#-------------------------------------------------------------------------------
# Dependencies:

$(BN)/$(MPRG): $(LB)/$(MPRG).o
				@$(LD) $(LDFLAGS) $< $(MLIBS) $(GLIBS) $(LIBS) -o $@
				@echo "Program $(MPRG) successfully linked... and starting..."


$(LB)/$(MPRG).o: $(MPRG).cxx
				@$(CXX) $(CXXFLAGS) -c $< -o $@
				@echo "Program $(MPRG) successfully compiled..."

#
#-------------------------------------------------------------------------------


