#----------------------------------------------------------------
#
#  Makefile for Eview (v2.0 - 09.09.2002)
#
#  Author: Andreas Zoglauer
#
#----------------------------------------------------------------



#----------------------------------------------------------------
# Definitions:
#

PRG = $(BN)/eview

MOBJS = $(LB)/MEviewMain.o

GOBJS  = $(LB)/MGUIEviewMain.o \
	$(LB)/MSettingsEview.o \
	$(LB)/MGUIEviewOptions.o \


OBJS = $(MOBJS) $(GOBJS)

SGOBJ = $(LB)/libEviewGui.$(DLL)

#----------------------------------------------------------------
# Commands:
#

all: $(PRG)

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
		@rm -f $(OBJS) $(SGOBJ)

#----------------------------------------------------------------
# Dependencies:
#

$(SGOBJ): $(GOBJS)
	@echo "Linking $(subst $(LB)/,,$@) ..."
ifeq ($(ARCH),macosx)
	$(LD) $(SOFLAGS) $(GOBJS) $(GLIBS) $(LIBS) -o $(SGOBJ)
	$(LD) -bundle -undefined suppress -Wl,-x $(LDFLAGS) $^ $(GLIBS) $(LIBS) \
	   -o $(subst .$(DLL),.so,$@)
else
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(GOBJS) $(GLIBS) $(LIBS) -o $(SGOBJ)
endif

$(OBJS): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(PRG): $(MOBJS) $(SGOBJ)
	@echo "Linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(EVIEWLIBS) $(GLIBS) $(LIBS) -o $(PRG)

#
#----------------------------------------------------------------
