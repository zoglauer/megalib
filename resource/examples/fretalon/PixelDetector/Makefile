#----------------------------------------------------------------
#
#  Makefile for Pixelizer
#
#  Top level makefile: definitions, paths and command switch
#
#  Author: Andreas Zoglauer
#
#----------------------------------------------------------------


SHELL = /bin/sh


#----------------------------------------------------------------
# Directories:
#

PIXELIZER_DIR = $(shell pwd)
IN = $(PIXELIZER_DIR)/include
LB = $(MEGALIB)/lib
BN = $(MEGALIB)/bin

#----------------------------------------------------------------
# Setting up make itself
#

include $(MEGALIB)/config/Makefile.options
include $(MEGALIB)/config/Makefile.config

MAKEFLAGS += --no-builtin-rules

.SUFFIXES:
#.SUFFIXES: .cxx .h .o .so
.PHONY: all n pixelizer megalib apps clean
.EXPORT_ALL_VARIABLES:
#.NOTPARALLEL: megalib
.SILENT:

#----------------------------------------------------------------
# Definitions based on architecture and user options
#

CMD=""
CXXFLAGS += -I$(IN) -I$(MEGALIB)/include -I/opt/local/include
# Comment this line out if you want to accept warnings
#CXXFLAGS += -Werror -Wno-unused-variable

# Name of the program
PIXELIZER_PRG = $(BN)/pixelizer

# The pixel detector library
PIXELIZER_LIBS = \
$(LB)/MAssembly.o \
$(LB)/MReadOutAssembly.o \
$(LB)/MModuleConverterSimRoa.o \


PIXELIZER_DEP_FILES := $(PIXELIZER_LIBS:.o=.d)
PIXELIZER_H_FILES := $(addprefix $(IN)/,$(notdir $(PIXELIZER_LIBS:.o=.h)))


FRETALON_DIR          := $(MEGALIB)/src/fretalon/framework
FRETALON_CXX_MAIN     := $(FRETALON_DIR)/src/MAssembly.cxx $(FRETALON_DIR)/src/MReadOutAssembly.cxx
FRETALON_CXX_FILES    := $(wildcard $(FRETALON_DIR)/src/*.cxx)
FRETALON_CXX_FILES    := $(filter-out $(FRETALON_CXX_MAIN),$(FRETALON_CXX_FILES))
FRETALON_H_FILES      := $(wildcard $(FRETALON_DIR)/inc/*.h)
FRETALON_H_FILES      := $(filter-out $(FRETALON_DIR)/inc/MAssembly.h $(FRETALON_DIR)/inc/MReadOutAssembly.h,$(FRETALON_H_FILES))
FRETALON_LIBS         := $(addprefix $(LB)/,$(notdir $(FRETALON_CXX_FILES:.cxx=.o)))
FRETALON_DEP_FILES    := $(FRETALON_LIBS:.o=.d)

# The shared library
PIXELIZER_SHARED_LIB = $(LB)/libPixelizer.$(DLL)

# The main program
PIXELIZER_CXX_MAIN := $(PIXELIZER_DIR)/src/MPixelizer.cxx

# External libraries
# MEGAlib
ALLLIBS = -L$(LB) -lResponseCreator -lFretalonBase -lSivan -lRevanGui -lRevan -lMimrec -lGeomega -lSpectralyzeGui -lSpectralyze -lCommonMisc -lCommonGui -L$(MEGALIB)/lib -L$(LB) 
# ROOT
ALLLIBS += -lMathCore



PIXELIZER_DICT_NAME=Pixelizer_Dictionary
PIXELIZER_DICT=$(LB)/$(PIXELIZER_DICT_NAME).cxx
PIXELIZER_DICT_LIB=$(LB)/$(PIXELIZER_DICT_NAME).o
PIXELIZER_LINKDEF=$(LB)/$(PIXELIZER_DICT_NAME)_LinkDef.h
PIXELIZER_ROOTMAP=$(LB)/libPixelizer.rootmap
PIXELIZER_ROOTPCM=libPixelizer_rdict.pcm

#----------------------------------------------------------------
# Command rules
#

all:
	@$(MAKE) $(PIXELIZER_PRG)
	@$(MAKE) apps

pixelizer:
	@$(MAKE) $(PIXELIZER_PRG)
	@$(PIXELIZER_PRG) $(CMD)

apps:
	@$(MAKE) $(PIXELIZER_SHARED_LIB)
	@$(MAKE) -C apps

clean:
	@-rm -f $(MEGALIB)/include/MAssembly.h $(MEGALIB)/include/MReadOutAssembly.h
	@-rm -f $(FRETALON_LIBS) $(FRETALON_DEP_FILES)
	@-rm -f $(PIXELIZER_SHARED_LIB) $(PIXELIZER_LIBS) $(PIXELIZER_DEP_FILES)
	@-rm -f $(PIXELIZER_PRG)
	@-rm -f $(PIXELIZER_DICT) $(PIXELIZER_LINKDEF) $(PIXELIZER_ROOTMAP) $(PIXELIZER_ROOTPCM)
	@-rm -f *~ include/*~ src/*~
	@$(MAKE) clean -C apps

#----------------------------------------------------------------
# Explicit rules & dependencies:
#

$(FRETALON_DEP_FILES): $(LB)/%.d: $(FRETALON_DIR)/src/%.cxx
	@echo "Creating dependencies for $(subst $(FRETALON_DIR)/src/,,$<) ..."
	@set -e; rm -f $@; $(CXX) $(DEPFLAGS) $(CXXFLAGS) $< > $@.$$$$; sed -e 's|.*:|$(LB)/$*.o:|' -e 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$

$(FRETALON_LIBS): $(LB)/%.o: $(FRETALON_DIR)/src/%.cxx $(FRETALON_DIR)/inc/%.h $(LB)/%.d
	@echo "Compiling $(subst $(FRETALON_DIR)/src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(PIXELIZER_DEP_FILES): $(LB)/%.d: src/%.cxx
	@echo "Creating dependencies for $(subst src/,,$<) ..."
	@set -e; rm -f $@; $(CXX) $(DEPFLAGS) $(CXXFLAGS) $< > $@.$$$$; sed -e 's|.*:|$(LB)/$*.o:|' -e 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$

$(PIXELIZER_LIBS): $(LB)/%.o: src/%.cxx include/%.h $(LB)/%.d
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(PIXELIZER_DICT): $(FRETALON_H_FILES) $(PIXELIZER_H_FILES)
	@echo "Generating LinkDef ..."
	@$(BN)/generatelinkdef -o $(PIXELIZER_LINKDEF) -i $(PIXELIZER_H_FILES) $(FRETALON_H_FILES)
	@echo "Generating dictionary ..."
	@rootcling -f $@ -I$(IN) -I$(MEGALIB)/include -D___CLING___ -rmf $(PIXELIZER_ROOTMAP) -s libPixelizer -c  $(PIXELIZER_H_FILES) $(FRETALON_H_FILES) $(PIXELIZER_LINKDEF)
	@mv $(PIXELIZER_ROOTPCM) $(LB)

$(PIXELIZER_DICT_LIB): $(PIXELIZER_DICT)
	@echo "Compiling dictionary ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(PIXELIZER_SHARED_LIB): $(FRETALON_LIBS) $(PIXELIZER_LIBS) $(PIXELIZER_DICT_LIB)
	@echo "Linking $(subst $(LB)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(PIXELIZER_DICT_LIB) $(PIXELIZER_LIBS) $(FRETALON_LIBS) $(GLIBS) $(LIBS) -o $(PIXELIZER_SHARED_LIB)

$(PIXELIZER_PRG): $(PIXELIZER_SHARED_LIB) $(PIXELIZER_CXX_MAIN)
	@echo "Linking and compiling $(subst $(BN)/,,$(PIXELIZER_PRG)) ... Please stand by ... "
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PIXELIZER_CXX_MAIN) $(PIXELIZER_SHARED_LIB) $(ALLLIBS) $(GLIBS) $(LIBS) -o $(PIXELIZER_PRG)

ifneq ($(MAKECMDGOALS),clean)
-include $(PIXELIZER_DEP_FILES)
-include $(FRETALON_DEP_FILES)
endif

#
#----------------------------------------------------------------
