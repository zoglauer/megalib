#----------------------------------------------------------------
#
#  Makefile for MEGAlib
#
#  Author: Andreas Zoglauer
#
#----------------------------------------------------------------



#----------------------------------------------------------------
# Definitions:
#
LOBJS      = $(LB)/MGUIDialog.o \
             $(LB)/MGUIElement.o \
             $(LB)/MGUIEEntry.o \
             $(LB)/MGUIEEntryList.o \
             $(LB)/MGUIEFileSelector.o \
             $(LB)/MGUIEMinMaxEntry.o \
             $(LB)/MGUIERBList.o \
             $(LB)/MGUIECBList.o \
             $(LB)/MGUIESlider.o \
             $(LB)/MGUIEStatusBar.o \
             $(LB)/MGUIEFlatToolBar.o \
             $(LB)/MGUIEFlatToolBarButton.o \
             $(LB)/MGUIEComboBox.o \
             $(LB)/MGUIETransceiver.o \
             $(LB)/MGUICBSelection.o \
             $(LB)/MGUIRBSelection.o \
             $(LB)/MGUIGeometry.o \
             $(LB)/MGUIMultiInput.o \
             $(LB)/MGUIProgressBar.o \
             $(LB)/MGUIAssistant.o \
             $(LB)/MGUIEText.o \
             $(LB)/MGUIETime.o \
             $(LB)/MGUIPrelude.o \
             $(LB)/MGUIAbout.o \
             $(LB)/MGUIMain.o \

SOBJ = $(LB)/libCommonGui.$(DLL)

#----------------------------------------------------------------
# Commands:
#

all: $(SOBJ) 

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
		@rm -f $(LOBJS) $(SOBJ)


#----------------------------------------------------------------
# Dependencies:
#

$(SOBJ): $(LOBJS)
	@echo "Linking $(subst $(LB)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) -lCommonMisc $(LOBJS) $(GLIBS) $(LIBS) -o $(SOBJ)

$(LOBJS): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@


#
#----------------------------------------------------------------



