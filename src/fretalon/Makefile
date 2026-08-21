#----------------------------------------------------------------
#
#  Makefile for Fretalon
#
#  Switches into the subdirectories of the source path
#
#  Author: Andreas Zoglauer
#
#----------------------------------------------------------------


#----------------------------------------------------------------
# Commands:
#

all:
	@$(MAKE) -C base
	@$(MAKE) -C melinator

unittests:
	@$(MAKE) -C base
	@$(MAKE) unittests -C framework

link:
	@$(MAKE) link -C base
	@$(MAKE) link -C melinator
	@$(MAKE) link -C framework

clean:
	@$(MAKE) clean -C base
	@$(MAKE) clean -C melinator
	@$(MAKE) clean -C framework

framework: all
	@$(MAKE) -C framework

clean_framework: 
	@$(MAKE) clean -C framework


#
#----------------------------------------------------------------
