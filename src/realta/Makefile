#----------------------------------------------------------------
#
#  Makefile for Realta
#
#  Author: Andreas Zoglauer
#
#----------------------------------------------------------------



#----------------------------------------------------------------
# Definitions:
#

PRG = $(BN)/realta

MOBJS     = $(LB)/MRealtaMain.o

LOBJS      = $(LB)/MSettingsRealta.o \
						$(LB)/MRealTimeAnalyzer.o \
						$(LB)/MRealTimeEvent.o \

GOBJS      = $(LB)/MGUIRealtaMain.o \
						$(LB)/MGUINetwork.o \
						$(LB)/MGUIAccumulation.o \

PRGT	= $(BN)/eventtransmitter 
MOBJT	= $(LB)/MEventTransmitter.o 

OBJS = $(MOBJS) $(LOBJS) $(GOBJS) $(MOBJT) 

SGOBJ = $(LB)/libRealtaGui.$(DLL)
SLOBJ = $(LB)/libRealta.$(DLL)


#----------------------------------------------------------------
# Commands:
#

all: $(PRG) $(PRGT)

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

$(MOBJS) $(GOBJS) $(LOBJS) $(MOBJT): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(PRG): $(MOBJS) $(SGOBJ) $(SLOBJ)
	@echo "Linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(MOBJS) $(REALTALIBS) $(GLIBS) $(LIBS) -o $(PRG)

$(PRGT): $(MOBJT) $(SGOBJ) $(SLOBJ)
	@echo "Linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(MOBJT) $(REALTALIBS) $(GLIBS) $(LIBS) -o $(PRGT)

#
#----------------------------------------------------------------



