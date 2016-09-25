//   Программа для замены таблицы разделов в загрузчике usbloader
// 
// 
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parts.h"

  
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
