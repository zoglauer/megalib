#----------------------------------------------------------------
#
#  Makefile for Sivan
#
#  Author: Andreas Zoglauer
#
#----------------------------------------------------------------



#----------------------------------------------------------------
# Definitions:
#

PRG = $(BN)/sivan

MOBJS     = $(LB)/MSivanMain.o

GOBJS      = $(LB)/MInterfaceSivan.o \
						$(LB)/MGUISivanMain.o \
						$(LB)/MGUISivanEventSelection.o \
						$(LB)/MGUISivanTracking.o \

LOBJS      = $(LB)/MSettingsSivan.o \
						$(LB)/MSimCluster.o \
						$(LB)/MSimHT.o \
						$(LB)/MSimIA.o \
						$(LB)/MSimDR.o \
						$(LB)/MSimGR.o \
						$(LB)/MSimPM.o \
						$(LB)/MSimEvent.o \
						$(LB)/MSimEventLoader.o \
						$(LB)/MFileEventsSim.o \

SGOBJ = $(LB)/libSivanGui.$(DLL)
SLOBJ = $(LB)/libSivan.$(DLL)

#----------------------------------------------------------------
# Commands:
#

all: $(PRG)

lib: $(SLOBJ)

link:
		@$(LINK) $(shell pwd)/inc/*.h $(IN)
		@rm -f $(IN)/LinkDef.h

clean:
		@rm -f src/*~
		@rm -f src/\#*
		@rm -f inc/*~
		@rm -f inc/\#*
		@rm -f *~
		@rm -f \#*
		@rm -f $(PRG)
		@rm -f $(MOBJS) $(GOBJS) $(LOBJS) $(SGOBJ) $(SLOBJ)


#----------------------------------------------------------------
# Dependencies:
#

$(SLOBJ): $(LOBJS)
	@echo "Linking $(subst $(LB)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(LOBJS) $(GLIBS) $(LIBS) -o $(SLOBJ)

$(SGOBJ): $(GOBJS) $(LOBJS)
	@echo "Linking $(subst $(LB)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(GOBJS) $(LOBJS) $(GLIBS) $(LIBS) -o $(SGOBJ)

$(MOBJS) $(GOBJS) $(LOBJS): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(PRG): $(MOBJS) $(SGOBJ) $(SLOBJ)
	@echo "Linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(SIVANLIBS) $(GLIBS) $(LIBS) -o $(PRG)

#
#----------------------------------------------------------------
