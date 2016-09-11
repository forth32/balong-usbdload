CC       = gcc
LIBS     = 
CFLAGS   = -O2 -g -Wno-unused-result

.PHONY: all clean

all:    balong-usbdload ptable-injector

clean: 
	rm *.o
	rm balong-usbdload
	rm ptable-injector
	
#.c.o:
#	$(CC) -o $@ $(LIBS) $^ qcio.o

balong-usbdload: balong-usbdload.o
	@gcc $^ -o $@ $(LIBS)

ptable-injector: ptable-injector.o
	@gcc $^ -o $@ $(LIBS)
	