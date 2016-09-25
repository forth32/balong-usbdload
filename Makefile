CC       = gcc
LIBS     = 
CFLAGS   = -O2 -g -Wno-unused-result

.PHONY: all clean

all:    balong-usbdload ptable-injector loader-patch

clean: 
	rm *.o
	rm balong-usbdload
	rm ptable-injector
	rm loader-patch
	
#.c.o:
#	$(CC) -o $@ $(LIBS) $^ qcio.o

balong-usbdload: balong-usbdload.o parts.o
	@gcc $^ -o $@ $(LIBS)

ptable-injector: ptable-injector.o parts.o
	@gcc $^ -o $@ $(LIBS)
	
loader-patch: loader-patch.o
	@gcc $^ -o $@ $(LIBS)
	