#-------------------------------------------------------------------------------
#
# Makefile for a stand alone application based on MEGAlib 
#
# Copyright (C) by Andreas Zoglauer.
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
PRG1=EventListCreator
PRG2=EventListSimCleaner
PRG3=EventListAnalyzer
PRG4=EventListFirstLook

# Basic directories
TOP = $(MEGALIB)
SR  = $(TOP)/src
IN  = $(TOP)/include
LB  = $(TOP)/lib
BN  = $(TOP)/bin
HT  = $(TOP)/doc/html
CT  = $(TOP)/cint

MPRG1=$(subst .cxx,,$(PRG1))
MPRG2=$(subst .cxx,,$(PRG2))
MPRG3=$(subst .cxx,,$(PRG3))
MPRG4=$(subst .cxx,,$(PRG4))

MLIBS = -L$(LB) -lMimrecGui -lMimrec -lRevanGui -lRevan -lSivanGui -lSivan -lSpectralyzeGui -lSpectralyze -lGeomegaGui -lGeomega -lCommonMisc -lCommonGui  


#-------------------------------------------------------------------------------
# Included options (result of ./configure)

include $(TOP)/config/Makefile.options
include $(TOP)/config/Makefile.config

CXXFLAGS += -I$(IN)


#-------------------------------------------------------------------------------
# Commands:


all: $(BN)/$(MPRG1) $(BN)/$(MPRG2) $(BN)/$(MPRG3) $(BN)/$(MPRG4)


only: $(BN)/$(MPRG1) $(BN)/$(MPRG2) $(BN)/$(MPRG3) $(BN)/$(MPRG4)

clean:
	rm -f $(LB)/$(MPRG1).o
	rm -f $(BN)/$(MPRG1)
	rm -f $(LB)/$(MPRG2).o
	rm -f $(BN)/$(MPRG2)
	rm -f $(LB)/$(MPRG3).o
	rm -f $(BN)/$(MPRG3)
	rm -f $(LB)/$(MPRG4).o
	rm -f $(BN)/$(MPRG4)


#-------------------------------------------------------------------------------
# Dependencies:

$(BN)/$(MPRG1): $(LB)/$(MPRG1).o
	@$(LD) $(LDFLAGS) $< $(MLIBS) $(GLIBS) $(LIBS) -o $@
	@echo "Program $(MPRG1) successfully linked... and starting..."

$(LB)/$(MPRG1).o: $(MPRG1).cxx
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "Program $(MPRG1) successfully compiled..."

	
$(BN)/$(MPRG2): $(LB)/$(MPRG2).o
	@$(LD) $(LDFLAGS) $< $(MLIBS) $(GLIBS) $(LIBS) -o $@
	@echo "Program $(MPRG2) successfully linked... and starting..."

$(LB)/$(MPRG2).o: $(MPRG2).cxx
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "Program $(MPRG2) successfully compiled..."

	
$(BN)/$(MPRG3): $(LB)/$(MPRG3).o
	@$(LD) $(LDFLAGS) $< $(MLIBS) $(GLIBS) $(LIBS) -o $@
	@echo "Program $(MPRG3) successfully linked... and starting..."

$(LB)/$(MPRG3).o: $(MPRG3).cxx
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "Program $(MPRG3) successfully compiled..."

	
$(BN)/$(MPRG4): $(LB)/$(MPRG4).o
	@$(LD) $(LDFLAGS) $< $(MLIBS) $(GLIBS) $(LIBS) -o $@
	@echo "Program $(MPRG4) successfully linked... and starting..."

$(LB)/$(MPRG4).o: $(MPRG4).cxx
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "Program $(MPRG4) successfully compiled..."

#
#-------------------------------------------------------------------------------


