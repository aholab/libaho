#DIRS=sys spl fsnd lpc10 graph misc 
DIRS=sys spl fsnd lpc10 misc 

obj:
	@$(foreach x, $(DIRS), $(MAKE) -C $(x)/source obj;)
#	@$(MAKE) -C extern obj

clean:
	@$(foreach x, $(DIRS), $(MAKE) -C $(x)/source clean;)
	@$(MAKE) -C extern clean

depend:
	@$(foreach x, $(DIRS), $(MAKE) -C $(x)/source depend;)

toux:
	@$(foreach x, $(DIRS), $(MAKE) -C $(x)/source toux;)

