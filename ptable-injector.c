//   Программа для замены таблицы разделов в загрузчике usbloader
// 
// 
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// сигнатура заголовка таблицы  
const uint8_t headmagic[16]={0x70, 0x54, 0x61, 0x62, 0x6c, 0x65, 0x48, 0x65, 0x61, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80};  

// Структура описателя раздела
struct ptable_line{
    char name[16];
    unsigned start;
    unsigned lsize;
    unsigned length;
    unsigned type;  
    unsigned loadaddr;   
    unsigned nproperty;  // флаги раздела
    unsigned entry;      
    unsigned count;
};

// Полная структура страницы таблицы разделов
struct ptable_t {
  uint8_t head[16];
  uint8_t version[16];
  uint8_t product[16];
  struct ptable_line part[41];
  uint8_t tail[32];
};

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
	 ptable.part[pnum].type,
	 ptable.part[pnum].entry,
	 ptable.part[pnum].nproperty,
	 ptable.part[pnum].count);
}
printf("\n");

}

  
//############################################################################################################3

void main(int argc, char* argv[]) {
  
  
int opt;  
int mflag=0;
char ptfile[100];
int rflag=0,xflag=0;

uint32_t ptaddr;
struct ptable_t ptable;

FILE* ldr;
FILE* out;
FILE* in;

while ((opt = getopt(argc, argv, "mr:hx")) != -1) {
  switch (opt) {
   case 'h': 
     
printf("\n Утилита для замены таблицы разделов в загрузчиках usbloader\
\n Модем должен находиться в режиме fastboot\
\n\n\
%s [ключи] <имя файла usbloader>\n\n\
 Допустимы следующие ключи:\n\n\
-m       - показать текущую карту разделов в usbloader\n\
-x       - извлечь текущую карту в файл ptable.bin\n\
-r <file>- заменить карту разделов на карту из указанного файла\n\
\n",argv[0]);
    return;
    
   case 'm':
    mflag=1;
    break;
    
   case 'x':
    xflag=1;
    break;
    
   case 'r':
     rflag=1;
     strcpy (ptfile,optarg);
     break;
     
   case '?':
   case ':':  
     return;
  
  }  
}  
if (optind>=argc) {
    printf("\n - Не указано имя файла загрузчика\n");
    return;
}  

ldr=fopen(argv[optind],"r+");
if (ldr == 0) {
  printf("\n Ошибка открытия файла %s\n",argv[optind]);
  return;
}

 
// Ищем таблицу разделов в файле загрузчика  

ptaddr=find_ptable(ldr);
if (ptaddr == 0) {
  printf("\n Таблица разделов в загрузчике не найдена\n");
  return ;
}
// читаем текущую таблицу
fread(&ptable,sizeof(ptable),1,ldr);

if (xflag) {
   out=fopen("ptable.bin","w");
   fwrite(&ptable,sizeof(ptable),1,out);
   fclose(out);
}   

if (mflag) {
  show_map(ptable);
}

if (mflag | xflag) return;

  
if (rflag) { 
  in=fopen(ptfile,"r");
  if (in == 0) {
    printf("\n Ошибка открытия файла %s",ptfile);
    return;
  }
  fread(&ptable,sizeof(ptable),1,in);
  fclose(in);
  
  // проверяем файл
  if (memcmp(ptable.head,headmagic,16) != 0) {
    printf("\n Входной файл не является таблицей разделов\n");
    return;
  }
  fseek(ldr,ptaddr,SEEK_SET);
  fwrite(&ptable,sizeof(ptable),1,ldr);
  fclose(ldr);
  
}  
}
