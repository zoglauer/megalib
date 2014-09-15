#----------------------------------------------------------------
#
#  Makefile for MEGAlib / fretalon
#
#  Author: Andreas Zoglauer
#
#----------------------------------------------------------------



#----------------------------------------------------------------
# Definitions:
#

PRG        := $(BN)/fretalon
CXX_MAIN   := src/MAssembly.cxx src/MReadOutAssembly.cxx
CXX_FILES  := $(wildcard src/*.cxx)
CXX_FILES  := $(filter-out $(CXX_MAIN),$(CXX_FILES))
H_FILES    := $(wildcard $(CURDIR)/inc/*.h)
H_FILES    := $(filter-out $(CURDIR)/inc/MAssembly.h $(CURDIR)/inc/MReadOutAssembly.h,$(H_FILES))
OBJ_FILES  := $(addprefix $(LB)/,$(notdir $(CXX_FILES:.cxx=.o)))
SOBJ       := $(LB)/libFretalon.$(DLL)

#----------------------------------------------------------------
# Commands:
#

.PHONY: fretalon

all: $(PRG)

lib: $(SOBJ) 

link:
	@$(LINK) $(H_FILES) $(IN)

clean:
	@rm -f src/*~
	@rm -f src/\#*
	@rm -f inc/*~
	@rm -f inc/\#*
	@rm -f *~
	@rm -f \#*
	@rm -f $(BN)/$(PRG)
	@rm -f $(OBJ_FILES) $(SOBJ)

#----------------------------------------------------------------
# Dependencies:
#


$(PRG): $(CXX_MAIN) $(SOBJ)
	@echo "Compiling and linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(CXXFLAGS) -Iinc $(LDFLAGS) $(CXX_MAIN) $(SOBJ) $(FRETALONLIBS) $(GLIBS) $(LIBS) -o $(PRG)

$(SOBJ): $(OBJ_FILES)
	@echo "Linking $(subst $(LB)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(OBJ_FILES) $(GLIBS) $(LIBS) -o $(SOBJ)

$(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -Iinc -c -o $@ $<

#
#----------------------------------------------------------------



