#----------------------------------------------------------------
#
#  Makefile Global
#
#  Author: Andreas Zoglauer
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



