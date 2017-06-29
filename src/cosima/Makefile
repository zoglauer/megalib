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

PRG        := $(BN)/cosima
CPP_MAIN   := src/MCCosima.cc
CPP_FILES  := $(wildcard src/*.cc)
CPP_FILES  := $(filter-out $(CPP_MAIN),$(CPP_FILES))
OBJ_FILES  := $(addprefix $(LB)/,$(notdir $(CPP_FILES:.cc=.o)))
SOBJ       := $(LB)/libCosima.$(DLL)
G4_CFLAGS  := $(shell geant4-config --cflags) -DG4VIS_USE
G4_CFLAGS  := $(filter-out -Wshadow, $(G4_CFLAGS))
G4_LIBS    := $(shell geant4-config --libs)


#----------------------------------------------------------------
# Commands:
#

all: $(PRG)

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
	@$(LD) $(CXXFLAGS) $(G4_CFLAGS) $(LDFLAGS) $(CPP_MAIN) $(COSIMALIBS) $(GLIBS) $(LIBS) $(G4_LIBS) -o $(PRG)

$(SOBJ): $(OBJ_FILES)
	@echo "Linking $(subst $(LB)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(OBJ_FILES) $(G4_LIBS) $(GLIBS) $(LIBS) -o $(SOBJ)

$(LB)/%.o: src/%.cc inc/%.hh
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) $(G4_CFLAGS) -c -o $@ $<


#
#----------------------------------------------------------------



