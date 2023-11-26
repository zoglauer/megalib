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
	@$(MAKE) link -C misc
	@$(MAKE) link -C gui

dict: dict_misc dict_gui

dict_misc:
	@$(MAKE) dict -C misc

dict_gui:
	@$(MAKE) dict -C gui

clean:
	@$(MAKE) clean -C misc
	@$(MAKE) clean -C gui
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



