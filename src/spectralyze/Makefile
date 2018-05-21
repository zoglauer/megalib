#----------------------------------------------------------------
#
#  Makefile for Spectral Analyzer
#
#  Author: Andreas Zoglauer and Michelle Galloway
#
#----------------------------------------------------------------



#----------------------------------------------------------------
# Base definitions:
#

PROGRAM = spectralyze

MAIN  = MSpectralyzerMain

LIBRARY = Spectralyze

FILES = MSettingsSpectralyzer \
	MSettingsSpectralOptions \
	MSpectralAnalyzer \
	MPeak \
	MQualifiedIsotope \


LIBRARY_UI = SpectralyzeGui

FILES_UI = \
	MGUISpectralAnalyzer \

LIBRARIES = \
	-lGeomega \
	-lCommonMisc \
	-lCommonGui \


#----------------------------------------------------------------
# Derived definitions:
#

OBJECTS=$(patsubst %, $(LB)/%.o, $(FILES))
HEADERS=$(patsubst %, inc/%.h, $(FILES))
SHAREDLIB=$(LB)/lib$(LIBRARY).$(DLL)

DICTIONARYNAME=$(LIBRARY)_Dictionary
DICTIONARY=$(LB)/$(DICTIONARYNAME).cxx
DICTIONARYOBJECT=$(LB)/$(DICTIONARYNAME).o
LINKDEF=$(LB)/$(DICTIONARYNAME)_LinkDef.h
ROOTMAP=$(LB)/lib$(LIBRARY).rootmap
ROOTPCM=lib$(LIBRARY)_rdict.pcm


OBJECTS_UI=$(patsubst %, $(LB)/%.o, $(FILES_UI))
HEADERS_UI=$(patsubst %, inc/%.h, $(FILES_UI))
SHAREDLIB_UI=$(LB)/lib$(LIBRARY_UI).$(DLL)

DICTIONARYNAME_UI=$(LIBRARY_UI)_Dictionary
DICTIONARY_UI=$(LB)/$(DICTIONARYNAME_UI).cxx
DICTIONARYOBJECT_UI=$(LB)/$(DICTIONARYNAME_UI).o
LINKDEF_UI=$(LB)/$(DICTIONARYNAME_UI)_LinkDef.h
ROOTMAP_UI=$(LB)/lib$(LIBRARY_UI).rootmap
ROOTPCM_UI=lib$(LIBRARY_UI)_rdict.pcm


OBJECT_MAIN=$(patsubst %, $(LB)/%.o, $(MAIN))
EXECUTABLE=$(BN)/$(PROGRAM)


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
	@rm -f $(OBJECTS_UI) $(SHAREDLIB_UI) $(DICTIONARY_UI) $(DICTIONARYOBJECT_UI) $(LINKDEF_UI) $(ROOTMAP_UI) $(LB)/$(ROOTPCM_UI)
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

$(SHAREDLIB): $(OBJECTS) $(DICTIONARYOBJECT)
	@echo "Linking lib$(LIBRARY) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(DICTIONARYOBJECT) $(OBJECTS) $(GLIBS) $(LIBS) -o $(SHAREDLIB)

$(OBJECTS): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(DICTIONARYOBJECT): $(DICTIONARY)
	@echo "Compiling dictionary ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@



$(DICTIONARY_UI): $(HEADERS_UI)
	@echo "Generating $(DICTIONARYNAME_UI)_LinkDef.h ..."
	@$(BN)/generatelinkdef -o $(LINKDEF_UI) -i $(HEADERS_UI)
	@echo "Generating dictionary $(DICTIONARYNAME_UI).cxx..."
	@rootcling -f $@ -I$(IN) -D___CLING___ -rmf $(ROOTMAP_UI) -s lib$(LIBRARY_UI) -c $^ $(LINKDEF_UI)
	@mv $(ROOTPCM_UI) $(LB)

$(SHAREDLIB_UI): $(OBJECTS_UI) $(DICTIONARYOBJECT_UI)
	@echo "Linking lib$(LIBRARY_UI) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(DICTIONARYOBJECT_UI) $(OBJECTS_UI) $(GLIBS) $(LIBS) -o $(SHAREDLIB_UI)

$(OBJECTS_UI): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(DICTIONARYOBJECT_UI): $(DICTIONARY_UI)
	@echo "Compiling dictionary ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@



$(OBJECT_MAIN): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(EXECUTABLE): $(SHAREDLIB_UI) $(SHAREDLIB) $(OBJECT_MAIN)
	@echo "Linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(OBJECT_MAIN) $(SHAREDLIB_UI) $(SHAREDLIB) $(LIBRARIES) $(GLIBS) $(LIBS) -o $(EXECUTABLE)


#
#----------------------------------------------------------------



