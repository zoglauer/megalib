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

link:
	@$(MAKE) link -C base
	@$(MAKE) link -C melinator
	@$(MAKE) link -C framework

dict: dict_base dict_melinator

dict_base:
	@$(MAKE) dict -C base

dict_melinator:
	@$(MAKE) dict -C melinator

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
