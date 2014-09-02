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
#	@$(MAKE) -C framework

link:
	@$(MAKE) link -C base
	@$(MAKE) link -C melinator
	@$(MAKE) link -C framework

clean:
	@$(MAKE) clean -C base
	@$(MAKE) clean -C melinator
#	@$(MAKE) clean -C framework


#
#----------------------------------------------------------------
