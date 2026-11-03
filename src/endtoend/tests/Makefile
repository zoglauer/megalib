#----------------------------------------------------------------
#
# Makefile of the end-to-end tests
#
#
# Every ET*.cxx in src/ is a program which is placed in bin/ and links the framework library libEndToEnd (../framework)
# The tests need the programs cosima, revan, and mimrec at run time (they fail, not skip, without them), the build only needs the libraries
#
# Copyright (C) by the MEGAlib contributors.
#
# This file is part of MEGAlib.
#
# MEGAlib is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# MEGAlib is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
# License (License.md) for more details.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
#
#----------------------------------------------------------------


#----------------------------------------------------------------
# Definitions:
#

LIBRARIES := \
	-lEndToEnd \
	-lSivan \
	-lGeomega \
	-lCommonMisc \
	-lCommonGui \

CXX_ET := $(wildcard src/ET*.cxx)
EXE_ET := $(patsubst src/%.cxx,$(BN)/%,$(CXX_ET))


#----------------------------------------------------------------
# Commands:
#

.PHONY: all tests lib link clean

all: $(EXE_ET)

tests: $(EXE_ET)

lib:

link:
	@$(LINK) $(shell pwd)/inc/*.h $(IN)

clean:
	@rm -f src/*~
	@rm -f src/\#*
	@rm -f inc/*~
	@rm -f inc/\#*
	@rm -f *~
	@rm -f \#*
	@rm -f $(EXE_ET)


#----------------------------------------------------------------
# Dependencies:
#

$(EXE_ET): $(BN)/%: src/%.cxx inc/%.h $(LB)/libEndToEnd.$(DLL)
	@echo "Compiling and linking $(subst $(BN)/,,$@) ..."
	@$(LD) $(CXXFLAGS) $(LDFLAGS) $< $(LIBRARIES) $(GLIBS) $(LIBS) -o $@

#
#----------------------------------------------------------------
