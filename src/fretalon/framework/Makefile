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
LIBRARY    := Fretalon
SOBJ       := $(LB)/lib$(LIBRARY).$(DLL)
LIBRARIES  := \
	-lFretalonBase \
	-lSivan \
	-lGeomega \
	-lCommonGui \
	-lCommonMisc \

DICTIONARYNAME:=$(LIBRARY)_Dictionary
DICTIONARY:=$(LB)/$(DICTIONARYNAME).cxx
DICTIONARYOBJECT:=$(LB)/$(DICTIONARYNAME).o
LINKDEF:=$(LB)/$(DICTIONARYNAME)_LinkDef.h
ROOTMAP:=$(LB)/lib$(LIBRARY).rootmap
ROOTPCM:=lib$(LIBRARY)_rdict.pcm

CXX_UT     := $(wildcard unittests/*.cxx)
EXE_UT     := $(patsubst %.cxx,%,$(CXX_UT))
EXE_UT     := $(patsubst unittests/%,$(BN)/%,$(EXE_UT))

#----------------------------------------------------------------
# Commands:
#

# unittests is also the name of a directory here, so the targets must be declared phony or make
# considers them up to date and skips their recipes
.PHONY: all unittests lib link headers clean

# link removes the two substituted headers and headers re-creates them, so nothing may be compiled
# until both have finished. Listing them as prerequisites is not enough: with a parallel make the
# compiles start alongside them. Recursive invocations force the order without serializing the
# compiles themselves.

all:
	@$(MAKE) link
	@$(MAKE) headers
	@$(MAKE) $(PRG)

unittests:
	@$(MAKE) link
	@$(MAKE) headers
	@$(MAKE) $(EXE_UT)

lib:
	@$(MAKE) link
	@$(MAKE) headers
	@$(MAKE) $(SOBJ)

link:
	@$(LINK) $(H_FILES) $(IN)
	@rm -f $(IN)/MAssembly.h $(IN)/MReadOutAssembly.h

headers: link
	@$(LINK) $(CURDIR)/inc/MAssembly.header $(IN)/MAssembly.h
	@$(LINK) $(CURDIR)/inc/MReadOutAssembly.header $(IN)/MReadOutAssembly.h
	
clean:
	@rm -f src/*~
	@rm -f src/\#*
	@rm -f inc/*~
	@rm -f inc/\#*
	@rm -f *~
	@rm -f \#*
	@rm -f $(addprefix $(IN)/,$(notdir $(H_FILES)))
	@rm -f $(IN)/MAssembly.h $(IN)/MReadOutAssembly.h
	@rm -f $(PRG)
	@rm -f $(OBJ_FILES) $(SOBJ) $(DEP_FILES) $(DICTIONARY) $(DICTIONARYOBJECT) $(LINKDEF) $(ROOTMAP) $(LB)/$(ROOTPCM)
	@rm -f $(EXE_UT)

#----------------------------------------------------------------
# Dependencies:
#

$(DICTIONARY): $(H_FILES)
	@echo "Generating $(DICTIONARYNAME)_LinkDef ..."
	@$(BN)/generatelinkdef -o $(LINKDEF) -i $(H_FILES)
	@echo "Generating dictionary ..."
	@rootcling -f $@ -I$(IN) -D___CLING___ -rmf $(ROOTMAP) -s lib$(LIBRARY) -c $^ $(LINKDEF)
	@mv $(ROOTPCM) $(LB)

$(DICTIONARYOBJECT): $(DICTIONARY)
	@echo "Compiling dictionary ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

	
$(PRG): $(CXX_MAIN) $(SOBJ)
	@echo "Compiling and linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(CXXFLAGS) -Iinc $(LDFLAGS) $(CXX_MAIN) $(SOBJ) $(LIBRARIES) $(GLIBS) $(LIBS) -o $(PRG)

$(SOBJ): $(DICTIONARYOBJECT) $(OBJ_FILES)
	@echo "Linking $(subst $(LB)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS)  $(DICTIONARYOBJECT) $(OBJ_FILES) $(GLIBS) $(LIBS) -o $(SOBJ)

$(OBJ_FILES): $(LB)/%.o: src/%.cxx inc/%.h #$(LB)/%.d
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -Iinc -c -o $@ $<
	
#$(DEP_FILES): $(LB)/%.d: src/%.cxx $(IN)/%.h
#	@echo "Creating dependencies for $(subst src/,,$<) ..."
#	@set -e; rm -f $@; $(CXX) -M $(CXXFLAGS) $< > $@.$$$$; sed -e 's|.*:|$(LB)/$*.o:|' -e 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$

#ifneq ($(MAKECMDGOALS),clean)
#-include $(DEP_FILES)
#endif 

$(EXE_UT): $(BN)/%: unittests/%.cxx src/MReadOutAssembly.cxx $(SOBJ)
	@echo "Compiling and linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(CXXFLAGS) -Iinc $(LDFLAGS) $< src/MReadOutAssembly.cxx $(SOBJ) $(LIBRARIES) $(GLIBS) $(LIBS) -o $@

# UTAssembly includes the application source directly to rename its main()
$(BN)/UTAssembly: src/MAssembly.cxx

#
#----------------------------------------------------------------



