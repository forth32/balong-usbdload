CC       = gcc
LIBS     = 
CFLAGS   = -O2 -g -Wno-unused-result

.PHONY: all clean

all:    balong-usbdload

clean: 
	rm *.o
	rm balong-usbdload

#.c.o:
#	$(CC) -o $@ $(LIBS) $^ qcio.o

balong-usbdload: balong-usbdload.o
	@gcc $^ -o $@ $(LIBS)
