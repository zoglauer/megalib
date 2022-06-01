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

PRG_MAIN   := $(BN)/cosima
CPP_MAIN   := src/MCCosima.cc

PRG_AUX    := $(BN)/GdmlConverter
CPP_AUX    := src/MCGdmlConverter.cc

CPP_FILES  := $(wildcard src/*.cc)
CPP_FILES  := $(filter-out $(CPP_MAIN) $(CPP_AUX),$(CPP_FILES))
OBJ_FILES  := $(addprefix $(LB)/,$(notdir $(CPP_FILES:.cc=.o)))
SOBJ       := $(LB)/libCosima.$(DLL)
G4_CFLAGS  := $(shell geant4-config --cflags) -DG4VIS_USE -Wno-deprecated-copy -Wno-overloaded-virtual
G4_CFLAGS  := $(filter-out -Wshadow, $(G4_CFLAGS))
G4_CFLAGS  := $(filter-out -Woverloaded-virtual, $(G4_CFLAGS))
G4_LIBS    := $(shell geant4-config --libs) -Wl,-rpath,$(shell geant4-config --prefix)/lib -lxerces-c
LIBRARIES  := -lSivan -lGeomega -lCommonMisc -lCommonGui


#----------------------------------------------------------------
# Commands:
#

all: $(PRG_MAIN) $(PRG_AUX)

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
	@rm -f $(OBJ_FILES) $(SOBJ)
	@rm -f $(PRG)
	@rm -f $(PRG_AUX)


#----------------------------------------------------------------
# Dependencies:
#

$(PRG_AUX): $(CPP_AUX) $(SOBJ)
	@echo "Compiling and linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(CXXFLAGS) $(G4_CFLAGS) $(LDFLAGS) $(CPP_AUX) $(SOBJ) $(LIBRARIES) $(GLIBS) $(LIBS) $(G4_LIBS) -o $(PRG_AUX)
	
$(PRG_MAIN): $(CPP_MAIN) $(SOBJ)
	@echo "Compiling and linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(CXXFLAGS) $(G4_CFLAGS) $(LDFLAGS) $(CPP_MAIN) $(SOBJ) $(LIBRARIES) $(GLIBS) $(LIBS) $(G4_LIBS) -o $(PRG_MAIN)

$(SOBJ): $(OBJ_FILES)
	@echo "Linking $(subst $(LB)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(OBJ_FILES) $(G4_LIBS) $(GLIBS) $(LIBS) -o $(SOBJ)

$(LB)/%.o: src/%.cc inc/%.hh
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) $(G4_CFLAGS) -c -o $@ $<


#
#----------------------------------------------------------------
