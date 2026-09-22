#----------------------------------------------------------------
#
#  Makefile Global
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



all:
	$(MAKE) -C misc
	$(MAKE) -C gui

link:
	@$(MAKE) link -C gui
	@$(MAKE) link -C misc

clean:
	@$(MAKE) clean -C gui
	@$(MAKE) clean -C misc
	@rm -f *~
	@rm -f \#*

clean_misc:
	@$(MAKE) clean -C misc
	@rm -f *~
	@rm -f \#*

clean_gui:
	@$(MAKE) clean -C gui
	@rm -f *~
	@rm -f \#*

clean_neuralnet:
	@$(MAKE) clean -C neuralnet
	@rm -f *~
	@rm -f \#*


#
#----------------------------------------------------------------



