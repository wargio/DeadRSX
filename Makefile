#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
all:
	@$(MAKE) -C libdeadrsx --no-print-directory
	@$(MAKE) -C samplesv2/background pkg --no-print-directory

clean:
	@$(MAKE) -C libdeadrsx clean --no-print-directory
	@$(MAKE) -C samplesv2/background clean --no-print-directory
