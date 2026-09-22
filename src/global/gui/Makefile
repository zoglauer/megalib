#----------------------------------------------------------------
#
#  Makefile for MEGAlib
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

LIBRARY := CommonGui

FILES := MGUIDefaults \
	MGUIDialog \
	MGUIElement \
	MGUIEEntry \
	MGUIEEntryList \
	MGUIEFileSelector \
	MGUIEMinMaxEntry \
	MGUIERBList \
	MGUIECBList \
	MGUIESlider \
	MGUIEStatusBar \
	MGUIEFlatToolBar \
	MGUIEFlatToolBarButton \
	MGUIEComboBox \
	MGUIETransceiver \
	MGUICBSelection \
	MGUIRBSelection \
	MGUIGeometry \
	MGUIMultiInput \
	MGUIProgressBar \
	MGUIMultiProgressBar \
	MGUIAssistant \
	MGUIEText \
	MGUIETime \
	MGUIPrelude \
	MGUIAbout \
	MGUIMain \


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

#----------------------------------------------------------------
# Commands:
#

all: $(SHAREDLIB)

link:
	@$(LINK) $(shell pwd)/inc/*.h $(IN)
	@echo "$(OBJECTS) $(DICTIONARYOBJECT)" >> $(LB)/AllObjects.txt

clean:
	@rm -f src/*~
	@rm -f src/\#*
	@rm -f inc/*~
	@rm -f inc/\#*
	@rm -f *~
	@rm -f \#*
	@rm -f $(OBJECTS) $(SHAREDLIB) $(DICTIONARY) $(DICTIONARYOBJECT) $(LINKDEF) $(ROOTMAP) $(LB)/$(ROOTPCM)

#----------------------------------------------------------------
# Dependencies:
#

$(DICTIONARY): $(HEADERS)
	@echo "Generating $(DICTIONARYNAME)_LinkDef ..."
	@$(BN)/generatelinkdef -o $(LINKDEF) -i $(HEADERS)
	@echo "Generating dictionary ..."
	@rootcling -f $@ -I$(IN) -D___CLING___ -rmf $(ROOTMAP) -s lib$(LIBRARY) -c $^ $(LINKDEF)
	@mv $(ROOTPCM) $(LB)
	@echo "Dictionary done ..."
	
$(SHAREDLIB): $(DICTIONARYOBJECT) $(OBJECTS) 
	@echo "Linking lib$(LIBRARY) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(DICTIONARYOBJECT) $(OBJECTS) $(GLIBS) $(LIBS) -o $(SHAREDLIB)

$(OBJECTS): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(DICTIONARYOBJECT): $(DICTIONARY)
	@echo "Compiling dictionary ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

#
#----------------------------------------------------------------



