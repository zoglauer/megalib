#----------------------------------------------------------------
#
#  Makefile for Response
#
#  Author: Andreas Zoglauer
#
#----------------------------------------------------------------



#----------------------------------------------------------------
# Definitions:
#

PRGM	= $(BN)/responsemanipulator 

MOBJM	=	$(LB)/MResponseManipulatorMain.o

PRGC	= $(BN)/responsecreator

MOBJC = $(LB)/MResponseCreatorMain.o

LOBJS = $(LB)/MResponseCreator.o \
				$(LB)/MResponseManipulator.o \
				$(LB)/MResponseBase.o \
				$(LB)/MResponseTracking.o \
				$(LB)/MResponseMultipleCompton.o \
				$(LB)/MResponseMultipleComptonLens.o \
				$(LB)/MResponseFirstInteractionPosition.o \
				$(LB)/MResponseSpectral.o \
				$(LB)/MResponseImaging.o \
				$(LB)/MResponseImagingARM.o \
				$(LB)/MResponseImagingBinnedMode.o \
				$(LB)/MResponseImagingCodedMask.o \
				$(LB)/MResponseEarthHorizon.o \


OBJS = $(MOBJM) $(MOBJC) $(LOBJS)

SLOBJ = $(LB)/libResponseCreator.$(DLL)

#----------------------------------------------------------------
# Commands:
#

all: $(PRGC) $(PRGM)

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
		@rm -f $(PRGC) $(PRGM)
		@rm -f $(MOBJC) $(MOBJM) $(LOBJS) $(SLOBJ)


#----------------------------------------------------------------
# Dependencies:
#

$(SLOBJ): $(LOBJS)
	@echo "Linking $(subst $(LB)/,,$@) ..."
ifeq ($(ARCH),macosx)
	$(LD) $(SOFLAGS) $(LOBJS) $(GLIBS) $(LIBS) -o $(SLOBJ)
	$(LD) -bundle -undefined suppress -Wl,-x $(LDFLAGS) $^ $(GLIBS) $(LIBS) \
	   -o $(subst .$(DLL),.so,$@)
else
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(LOBJS) $(GLIBS) $(LIBS) -o $(SLOBJ)
endif


$(OBJS): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(PRGM): $(MOBJM) $(SLOBJ)
	@echo "Linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(MOBJM) $(LOBJS) $(RESPONSELIBS) $(GLIBS) $(LIBS) -o $(PRGM)

$(PRGC): $(MOBJC) $(SLOBJ)
	@echo "Linking $(subst $(BN)/,,$@)..."
	@$(LD) $(LDFLAGS) $(MOBJC) $(LOBJS) $(RESPONSELIBS)  $(GLIBS) $(LIBS) -o $(PRGC)

#
#----------------------------------------------------------------



