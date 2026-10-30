#----------------------------------------------------------------
#
#  Makefile for Eview (v2.0 - 09.09.2002)
#
#  Copyright (C) by the MEGAlib contributors.
#
#  This file is part of MEGAlib.
#
#  MEGAlib is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Lesser General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  MEGAlib is distributed in the hope that it will be useful, but WITHOUT
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
#  License (License.md) for more details.
#
#  SPDX-License-Identifier: LGPL-3.0-or-later
#
#----------------------------------------------------------------



#----------------------------------------------------------------
# Base definitions:
#

PROGRAM := eview

MAIN    := MEviewMain

LIBRARY := Eview

FILES := MGUIEviewMain \
	MSettingsEview \
	MGUIEviewOptions \

LIBRARIES := \
	-lRevan \
	-lRevanGui \
	-lSivan \
	-lSpectralyze \
	-lSpectralyzeGui \
	-lGeomega \
	-lCommonMisc \
	-lCommonGui \

#----------------------------------------------------------------
# Derived definitions:
#

OBJECTS:=$(patsubst %, $(LB)/%.o, $(FILES))
HEADERS:=$(patsubst %, inc/%.h, $(FILES))
SHAREDLIB:=$(LB)/lib$(LIBRARY).$(DLL)

DICTIONARYNAME:=$(LIBRARY)_Dictionary
DICTIONARY:=$(LB)/$(DICTIONARYNAME).cxx
DICTIONARYOBJECT:=$(LB)/$(DICTIONARYNAME).o
LINKDEF:=$(LB)/$(DICTIONARYNAME)_LinkDef.h
ROOTMAP:=$(LB)/lib$(LIBRARY).rootmap
ROOTPCM:=lib$(LIBRARY)_rdict.pcm


OBJECT_MAIN:=$(patsubst %, $(LB)/%.o, $(MAIN))
EXECUTABLE:=$(BN)/$(PROGRAM)


#----------------------------------------------------------------
# Commands:
#

all: $(EXECUTABLE)

lib: $(SHAREDLIB) 

link:
	@$(LINK) $(shell pwd)/inc/*.h $(IN)
	@echo "$(OBJECTS) $(DICTIONARYOBJECT) $(OBJECTS_UI) $(DICTIONARYOBJECT_UI)" >> $(LB)/AllObjects.txt

clean:
	@rm -f src/*~
	@rm -f src/\#*
	@rm -f inc/*~
	@rm -f inc/\#*
	@rm -f *~
	@rm -f \#*
	@rm -f $(OBJECTS) $(SHAREDLIB) $(DICTIONARY) $(DICTIONARYOBJECT) $(LINKDEF) $(ROOTMAP) $(LB)/$(ROOTPCM)
	@rm -f $(OBJECTS_MAIN) $(EXECUTABLE)

	
#----------------------------------------------------------------
# Dependencies:
#

$(DICTIONARY): $(HEADERS)
	@echo "Generating $(DICTIONARYNAME)_LinkDef.h ..."
	@$(BN)/generatelinkdef -o $(LINKDEF) -i $(HEADERS)
	@echo "Generating dictionary $(DICTIONARYNAME).cxx..."
	@rootcling -f $@ -I$(IN) -D___CLING___ -rmf $(ROOTMAP) -s lib$(LIBRARY) -c $^ $(LINKDEF)
	@mv $(ROOTPCM) $(LB)

$(SHAREDLIB): $(DICTIONARYOBJECT) $(OBJECTS)
	@echo "Linking lib$(LIBRARY) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(DICTIONARYOBJECT) $(OBJECTS) $(GLIBS) $(LIBS) -o $(SHAREDLIB)

$(OBJECTS): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(DICTIONARYOBJECT): $(DICTIONARY)
	@echo "Compiling dictionary ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@



$(OBJECT_MAIN): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(EXECUTABLE): $(SHAREDLIB) $(OBJECT_MAIN)
	@echo "Linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(OBJECT_MAIN) $(SHAREDLIB) $(LIBRARIES) $(GLIBS) $(LIBS) -o $(EXECUTABLE)

	
#
#----------------------------------------------------------------
