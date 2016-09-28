#include <stdio.h>
#include <stdint.h>
#ifndef WIN32
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#else
#include <windows.h>
#include "printf.h"
#endif

#include "parts.h"

// сигнатура заголовка таблицы  
const uint8_t headmagic[16]={0x70, 0x54, 0x61, 0x62, 0x6c, 0x65, 0x48, 0x65, 0x61, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80};  

//*********************************************
//* Поиск таблицы разделов в загрузчике 
//*********************************************
uint32_t find_ptable(FILE* ldr) {

uint8_t rbuf[16];
while (fread(rbuf,1,16,ldr) == 16) {
  if (memcmp(rbuf,headmagic,16) == 0) {
    fseek(ldr,-16,SEEK_CUR);
    return ftell(ldr);
  }
  fseek(ldr,-12,SEEK_CUR);
}
return 0;
}
  
//*********************************************
//* Печать таблицы разделов
//*********************************************
void show_map(struct ptable_t ptable) {

int pnum;
  
printf("\n Версия таблицы разделов: %16.16s",ptable.version);
printf("\n Версия прошивки:         %16.16s\n",ptable.product);

printf("\n ## ----- NAME ----- start  len  loadsize loadaddr  entry    flags    type     count\n------------------------------------------------------------------------------------------");

for(pnum=0;
   (ptable.part[pnum].name[0] != 0) &&
   (strcmp(ptable.part[pnum].name,"T") != 0);
   pnum++) {

   printf("\n %02i %-16.16s %4x  %4x  %08x %08x %08x %08x %08x %08x",
	 pnum,
	 ptable.part[pnum].name,
	 ptable.part[pnum].start/0x20000,
	 ptable.part[pnum].length/0x20000,
	 ptable.part[pnum].lsize,
	 ptable.part[pnum].loadaddr,
	 ptable.part[pnum].entry,
	 ptable.part[pnum].nproperty,
	 ptable.part[pnum].type,
	 ptable.part[pnum].count);
}
printf("\n");

}

//*********************************************
//* Поиск таблицы разделов в памяти
//*********************************************
uint32_t find_ptable_ram(char* buf, uint32_t size) {

// сигнатура заголовка таблицы  
uint32_t off;

for(off=0;off<(size-16);off+=4) {
  if (memcmp(buf+off,headmagic,16) == 0)   return off;
}
return 0;
}

