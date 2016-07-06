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
DEP_FILES  := $(OBJ_FILES:.o=.d)
SOBJ       := $(LB)/libFretalon.$(DLL)

#----------------------------------------------------------------
# Commands:
#

.PHONY: fretalon

all: link $(PRG)

lib: link $(SOBJ) 

link:
	@$(LINK) $(H_FILES) $(IN)

clean:
	@rm -f src/*~
	@rm -f src/\#*
	@rm -f inc/*~
	@rm -f inc/\#*
	@rm -f *~
	@rm -f \#*
	@rm -f $(addprefix $(IN)/,$(notdir $(H_FILES)))
	@rm -f $(BN)/$(PRG)
	@rm -f $(OBJ_FILES) $(SOBJ) $(DEP_FILES)

#----------------------------------------------------------------
# Dependencies:
#


$(PRG): $(CXX_MAIN) $(SOBJ)
	@echo "Compiling and linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(CXXFLAGS) -Iinc $(LDFLAGS) $(CXX_MAIN) $(SOBJ) $(FRETALONLIBS) $(GLIBS) $(LIBS) -o $(PRG)

$(SOBJ): $(OBJ_FILES)
	@echo "Linking $(subst $(LB)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(OBJ_FILES) $(GLIBS) $(LIBS) -o $(SOBJ)

$(OBJ_FILES): $(LB)/%.o: src/%.cxx inc/%.h #$(LB)/%.d
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -Iinc -c -o $@ $<
	
#$(DEP_FILES): $(LB)/%.d: src/%.cxx $(IN)/%.h
#	@echo "Creating dependencies for $(subst src/,,$<) ..."
#	@set -e; rm -f $@; $(CXX) -M $(CXXFLAGS) $< > $@.$$$$; sed -e 's|.*:|$(LB)/$*.o:|' -e 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$

#ifneq ($(MAKECMDGOALS),clean)
#-include $(DEP_FILES)
#endif 

#
#----------------------------------------------------------------



