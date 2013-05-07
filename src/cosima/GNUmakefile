#----------------------------------------------------------------
# Makefile for Cosima
# (C) by Andreas Zoglauer
#----------------------------------------------------------------


# Name of the program
name := cosima
G4TARGET := $(name)
G4EXLIB := true

G4TMP = $(MEGALIB)/lib
G4BIN = $(MEGALIB)/bin
G4WORKDIR = $(MEGALIB)

# Parameters to be passed to the program
CMD=

# The index-file
INDEX = doc/html/index.html


.PHONY: all only

all: lib bin
	@if ( test ! -h $(BN)/$(name) ); then cd $(BN); ln -s $(G4SYSTEM)/$(name) $(name); fi
	@if ( test ! -h $(LB)/libcosima.$(DLL) ); then cd $(LB); ln -s $(G4SYSTEM)/$(name)/libcosima.$(DLL) libcosima.$(DLL); fi

only: lib bin

debug: only
		gdb bin/$(G4SYSTEM)/$(name) -x resource/gdb.ini

cos: all
		@bin/$(G4SYSTEM)/$(name) $(CMD)

remove:
		@$(MAKE) clean -C doc 
		@rm -f bin/$(G4SYSTEM)/$(name)

htmldoc: $(INDEX)
		rm -f doc/html/*
		sh resource/zogy2doxy
		doxygen resource/Doxyfile

texdoc:
		@acroread doc/$(name).pdf &

changelog:
		@sh $(MEGALIB)/bin/changelog.sh
		@more doc/ChangeLog

$(INDEX): src/*.cc  include/*.hh


# Load global G4 parameters:
include $(G4INSTALL)/config/architecture.gmk
include $(G4INSTALL)/config/binmake.gmk

# For some stupid reasons we have to include this here...
LDLIBS += -L$(G4LIB)/$(G4SYSTEM)


#----------------------------------------------------------------
# Integration of ROOT & MEGAlib:

# Extensions
DLL := $(shell grep DLL $(MEGALIB)/config/Makefile.config | cut -d "=" -f 2)
DLL := $(strip $(DLL))

ifndef G4UI_USE_ROOT
CPPFLAGS += -DG4ANALYSIS_USE_ROOT $(shell root-config --cflags) -I$(MEGALIB)/include -Wno-long-long
ROOTLIBS = $(shell root-config --nonew --glibs) -lMinuit -lHtml -lThread
ROOTLIBS := $(filter-out -lNew,$(ROOTLIBS))
#ROOTLIBS := $(filter-out -lThread,$(ROOTLIBS))
ROOTLIBS := $(filter-out -lpthread,$(ROOTLIBS))
LDLIBS += $(ROOTLIBS) $(MEGALIB)/lib/libSivan.$(DLL) $(MEGALIB)/lib/libGeomega.$(DLL) $(MEGALIB)/lib/libCommonMisc.$(DLL) $(MEGALIB)/lib/libCommonGui.$(DLL) 
endif


#----------------------------------------------------------------
