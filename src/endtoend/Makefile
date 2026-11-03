#----------------------------------------------------------------
#
# Makefile of the end-to-end testing: the framework (library libEndToEnd with the test driver) and the tests
#
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


.PHONY: all tests lib link clean

all:
	@$(MAKE) -C framework
	@$(MAKE) -C tests

tests:
	@$(MAKE) tests -C framework
	@$(MAKE) tests -C tests

lib:
	@$(MAKE) lib -C framework

link:
	@$(MAKE) link -C framework
	@$(MAKE) link -C tests

clean:
	@$(MAKE) clean -C framework
	@$(MAKE) clean -C tests
