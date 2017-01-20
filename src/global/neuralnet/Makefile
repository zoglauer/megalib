#----------------------------------------------------------------
#
#  Makefile for neural net base classes
#
#  Author: Andreas Zoglauer
#
#----------------------------------------------------------------



#----------------------------------------------------------------
# Definitions:
#

PRGT	= $(BN)/neuralnettester

TOBJS	= $(LB)/Main.o

NOBJS 	= $(LB)/MNeuron.o \
	$(LB)/MAdalineNeuron.o \
	$(LB)/MBiasNeuron.o \
	$(LB)/MInputNeuron.o \
	$(LB)/MFeedForwardNeuron.o \
	$(LB)/MBackpropagationNeuron.o \
	$(LB)/MBackpropagationMiddleNeuron.o \
	$(LB)/MBackpropagationOutputNeuron.o \
	$(LB)/MSynapse.o \
	$(LB)/MAdalineSynapse.o \
	$(LB)/MBackpropagationSynapse.o \
	$(LB)/MNeuralNetwork.o \
	$(LB)/MNeuralNetworkIOStore.o \
	$(LB)/MNeuralNetworkBackpropagation.o \
	$(LB)/MNeuralNetworkBackpropagationAssembly.o \

OBJS	= $(NOBJS) $(TOBJS)

SNOBJ	= $(LB)/libNeuralNet.$(DLL)



#----------------------------------------------------------------
# Command rules:
#

all: $(PRGT)

lib: $(SNOBJ)

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
	@rm -f $(PRGT)
	@rm -f $(TOBJS) $(NOBJS) $(SOBJS)


#----------------------------------------------------------------
# Dependencies:
#

$(OBJS): $(LB)/%.o: src/%.cxx inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(SNOBJ): $(NOBJS)
	@echo "Linking $(subst $(LB)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(NOBJS) $(GLIBS) $(LIBS) -o $(SNOBJ)

$(PRGT): $(TOBJS) $(SNOBJ)
	@echo "Linking $(subst $(BN)/,,$@)..."
	@$(LD) $(LDFLAGS) $(TOBJS) $(SNOBJ) -lCommonMisc -lCommonGui $(GLIBS) $(LIBS) -o $(PRGT)

#
#----------------------------------------------------------------



