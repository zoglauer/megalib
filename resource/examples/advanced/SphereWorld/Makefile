#----------------------------------------------------------------
#
#  Makefile for MEGAlib / cosima
#
#  Author: Andreas Zoglauer
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

PRG1       := sphereworldsimulator
PRG1_MAIN  := SphereWorldSimulator.cxx
PRG2       := cellviewer
PRG2_MAIN  := CellViewer.cxx
PRG3       := cellmerger
PRG3_MAIN  := CellMerger.cxx
PRG4       := feeder
PRG4_MAIN  := Feeder.cxx

G4_CFLAGS  := $(shell geant4-config --cflags)
G4_LIBS    := $(shell geant4-config --libs)

MLIBS      := -L$(LB) -lCosima -lMimrecGui -lMimrec -lRevanGui -lRevan -lSivanGui -lSivan -lSpectralyzeGui -lSpectralyze -lGeomegaGui -lGeomega -lCommonMisc -lCommonGui  

include $(TOP)/config/Makefile.options
include $(TOP)/config/Makefile.config

CXXFLAGS += -I$(IN)


#----------------------------------------------------------------
# Commands:
#

all: $(PRG1) $(PRG2) $(PRG3) $(PRG4)


#----------------------------------------------------------------
# Dependencies:
#

$(PRG1) : $(PRG1_MAIN)
	@echo "Compiling and linking $(PRG1) ..."
	@$(LD) $(CXXFLAGS) $(G4_CFLAGS) $(LDFLAGS) $(PRG1_MAIN) $(MLIBS) $(GLIBS) $(LIBS) $(G4_LIBS) -o $(BN)/$(PRG1)

$(PRG2): $(PRG2_MAIN)
	@echo "Compiling and linking $(PRG2) ..."
	@$(LD) $(CXXFLAGS) $(LDFLAGS) $(PRG2_MAIN) $(MLIBS) $(GLIBS) $(LIBS) $(G4_LIBS) -o $(BN)/$(PRG2)

$(PRG3): $(PRG3_MAIN)
	@echo "Compiling and linking $(PRG3) ..."
	@$(LD) $(CXXFLAGS) $(LDFLAGS) $(PRG3_MAIN) $(MLIBS) $(GLIBS) $(LIBS) $(G4_LIBS) -o $(BN)/$(PRG3)

$(PRG4): $(PRG4_MAIN)
	@echo "Compiling and linking $(PRG4) ..."
	@$(LD) $(CXXFLAGS) $(LDFLAGS) $(PRG4_MAIN) $(MLIBS) $(GLIBS) $(LIBS) $(G4_LIBS) -o $(BN)/$(PRG4)


#
#----------------------------------------------------------------



