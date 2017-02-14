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
  
  

struct ptable_t ptable;
FILE* in;

if (argc != 2) {
    printf("\n - Не указано имя файла с таблицей разделов\n");
    return;
}  

in=fopen(argv[optind],"r+");
if (in == 0) {
  printf("\n Ошибка открытия файла %s\n",argv[optind]);
  return;
}

 
// читаем текущую таблицу
fread(&ptable,sizeof(ptable),1,in);

if (strncmp(ptable.head, "pTableHead", 16) != 0) {
  printf("\n Файл не является таблицей разделов\n");
  return ;
}
  
show_map(ptable);
}
