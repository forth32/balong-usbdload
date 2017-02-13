CC       = gcc
LIBS     = 
CFLAGS   = -O2 -g -Wno-unused-result

.PHONY: all clean

all:    balong-usbdload ptable-injector loader-patch ptable-list

clean: 
	rm *.o
	rm balong-usbdload
	rm ptable-injector
	rm loader-patch
	rm ptable-list
	
#.c.o:
#	$(CC) -o $@ $(LIBS) $^ qcio.o

balong-usbdload: balong-usbdload.o parts.o patcher.o
	@gcc $^ -o $@ $(LIBS)

ptable-injector: ptable-injector.o parts.o
	@gcc $^ -o $@ $(LIBS)
	
loader-patch: loader-patch.o patcher.o
	@gcc $^ -o $@ $(LIBS)

ptable-list: ptable-list.o parts.o
	@gcc $^ -o $@ $(LIBS)
	