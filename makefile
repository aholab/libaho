
.NOTPARALLEL:
LIBNAME=libaho32.a
LIBNAMED=libahod32.a
LIBNAME64=libaho.a
LIBNAME64D=libahod.a


all: lib64 lib64d lib libd includes

lib:$(LIBNAME)

lib64:$(LIBNAME64)

libd:$(LIBNAMED)

lib64d:$(LIBNAME64D)


#DIRS=sys spl fsnd lpc10 misc graph
DIRS=sys spl fsnd lpc10 misc



$(LIBNAME):
	$(MAKE) clean
	@export DEBUG=n && export BITS32=y && $(MAKE) -C src obj
	ar rv lib/$(LIBNAME) $(foreach x, $(DIRS), src/$(x)/source/*.o)  # src/extern/*.o
	@ranlib lib/$(LIBNAME)

$(LIBNAME64):
	$(MAKE) clean
	@export DEBUG=n && export BITS32=n &&  $(MAKE) -C src obj
	ar rv lib/$(LIBNAME64) $(foreach x, $(DIRS), src/$(x)/source/*.o)  # src/extern/*.o
	@ranlib lib/$(LIBNAME64)

$(LIBNAMED):
	$(MAKE) clean
	@export DEBUG=y && export BITS32=y && $(MAKE) -C src obj
	ar rv lib/$(LIBNAMED) $(foreach x, $(DIRS), src/$(x)/source/*.o) # src/extern/*.o
	@ranlib lib/$(LIBNAMED)

$(LIBNAME64D):
	$(MAKE) clean
	@export DEBUG=y && export BITS32=n && $(MAKE) -C src obj
	ar rv lib/$(LIBNAME64D) $(foreach x, $(DIRS), src/$(x)/source/*.o) # src/extern/*.o
	@ranlib lib/$(LIBNAME64D)
includes:
	@cd include;chmod +w .;ln -sf $(foreach x, $(DIRS) ., ../src/$(x)/include/*.h*) .;chmod -w .
#	@cd libdos/include;chmod +w .;ln -sf ../../include/* ../../src/dos/include/*.h* .;chmod -w .
#	@cd libdos/sinclude;chmod +w .;ln -sf $(foreach x, sys spl fsnd lpc10, ../../src/$(x)/source/*.h*) .;chmod -w .
#	@cd libdjgpp/include;chmod +w .;ln -sf ../../include/* ../../src/dos/include/*.h* .;chmod -w .
#	@cd libdjgpp/sinclude;chmod +w .;ln -sf $(foreach x, sys spl fsnd lpc10, ../../src/$(x)/source/*.h*) .;chmod -w .
#	@cd libwin/include;chmod +w .;ln -sf ../../include/* .;chmod -w .
#	@cd libwin/sinclude;chmod +w .;ln -sf $(foreach x, sys spl fsnd lpc10, ../../src/$(x)/source/*.h*) .;chmod -w .

docs:
	@misc/mkdoc

obj:
	@$(MAKE) -C src obj

clean:
	@$(MAKE) -C src clean

cleanall:clean
	@rm -f lib/$(LIBNAME) lib/$(LIBNAMED)
	@chmod +w include 
	@chmod +w libdos/include libdos/sinclude 
	@chmod +w libwin/include libwin/sinclude 
	@rm -f doc/*
	@rm -f include/* libdos/include/* libdos/sinclude/*
	@rm -f include/* libwin/include/* libwin/sinclude/*
	@rm -f libdos/*.dsk libdos/release/* libdos/debug/*
	@rm -f libwin/*.mak libwin/release/* libwin/debug/*
	@rm -f libwin/*.ncb libwin/*.opt libwin/*.plg

depend:
	@$(MAKE) -C src depend

toux:
	@$(MAKE) -C src toux
