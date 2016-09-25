#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

// Структура, описывающая сигнатуру и положение патча
struct defpatch {
 const char* sig; // сигнатрура
 uint32_t sigsize; // длина сигнатуры
 int32_t poffset;  // смещение до точки патча от конца сигнатуры
};



//***********************************************************************
//* Поиск сигнатуры и наложение патча
//***********************************************************************
uint32_t patch(struct defpatch fp, uint8_t* buf, uint32_t fsize) {

// накладываемый патч - mov r0,#0
const char nop0[4]={0, 0, 0xa0, 0xe3};   
uint32_t i;

for(i=8;i<(fsize-60);i+=4) {
  if (memcmp(buf+i,fp.sig, fp.sigsize) == 0) {
    // найдена сигнатура - накладываем патч и уходим
    memcpy(buf+i+fp.sigsize+fp.poffset,nop0,4);
    return i;
  }
}
// сигнатрура не найдена
return 0;
}



//#######################################################################################################
void main(int argc, char* argv[]) {
  
FILE* in;
FILE* out;
uint8_t* buf;
uint32_t fsize;
int opt;
uint8_t outfilename[100];
int oflag=0,bflag=0;
uint32_t res;

// Описатели патчей

const char sigburn_v7r11[]={
   0x00, 0x00, 0x50, 0xE3, 0x70, 0x80, 0xBD, 0x08, 0x00, 0x30, 0xA0, 0xE3, 
   0x4E, 0x26, 0x04, 0xE3, 0xE0, 0x3F, 0x44, 0xE3, 0x55, 0x22, 0x44, 0xE3, 
   0x4C, 0x01, 0x9F, 0xE5, 0x4E, 0xC6, 0x04, 0xE3, 0x48, 0x41, 0x9F, 0xE5, 
   0x02, 0x10, 0xA0, 0xE1, 0x4C, 0xC4, 0x44, 0xE3, 0x5C, 0x23, 0x83, 0xE5, 
   0x00, 0x00, 0x8F, 0xE0, 0x40, 0xC3, 0x83, 0xE5};

const char sigburn_v7r2[]={   
   0x00, 0x30, 0xA0, 0xE3, 0x9A, 0x3F, 0x07, 0xEE, 0xFF, 0x2F, 0x0F, 0xE3,
   0xE1, 0x2F, 0x44, 0xE3, 0x07, 0x30, 0x02, 0xE5, 0x9A, 0x3F, 0x07, 0xEE, 
   0x00, 0x40, 0xA0, 0xE3, 0xE0, 0x4F, 0x44, 0xE3, 0x4E, 0x36, 0x04, 0xE3, 
   0x4C, 0x34, 0x44, 0xE3, 0x30, 0x33, 0x84, 0xE5};
 
// 00 30 A0 E3 9A 3F 07 EE FF 2F 0F E3 07 30 02 E5 9A 3F 07 EE 00 40 A0 E3 4E 36 04 E3 30 33 84 E5    
   
const char sigbad[]={0x04, 0x10, 0x8D, 0xE2, 0x04, 0x00, 0xA0, 0xE1};

struct defpatch patch_v7r11={sigburn_v7r11, sizeof(sigburn_v7r11), 4};   
struct defpatch patch_v7r2={sigburn_v7r2, sizeof(sigburn_v7r2), 16};   
struct defpatch patch_erasebad={sigbad, sizeof(sigbad), 0};   
   

// Разбор командной строки

while ((opt = getopt(argc, argv, "o:bh")) != -1) {
  switch (opt) {
   case 'h': 
     
printf("\n Программа для автоматического патча загрузчиков платформ Balong V7\n\n\
%s [ключи] <файл загрузчика usbloader>\n\n\
 Допустимы следующие ключи:\n\n\
-o file  - имя выходного файла. По умолчанию производится только проверка возможности патча\n\
-b       - добавить патч, отключающий проверку дефектных блоков\n\
\n",argv[0]);
    return;

   case 'o':
    strcpy(outfilename,optarg);
    oflag=1;
    break;

   case 'b':
     bflag=1;
     break;
     
   case '?':
   case ':':  
     return;
    
  }
}  

printf("\n Программа автоматической модификации загрузчиков Balong V7, (c) forth32");

 if (optind>=argc) {
    printf("\n - Не указано имя файла для загрузки\n - Для подсказки укажите ключ -h\n");
    return;
}  
    
in=fopen(argv[optind],"r");
if (in == 0) {
  printf("\n Ошибка открытия файла %s",argv[optind]);
  return;
}

// определяем размер файла
fseek(in,0,SEEK_END);
fsize=ftell(in);
rewind(in);

// выделяем буфер и читаем туда весь файл
buf=malloc(fsize);
fread(buf,1,fsize,in);
fclose(in);

res=patch(patch_v7r2, buf, fsize);
if (res != 0)  printf("\n* Найдена сигнатура типа V7R2 по смещению %08x",res);
else {
   res=patch(patch_v7r11, buf, fsize);
   if (res != 0)  printf("\n* Найдена сигнатура типа V7R11 по смещению %08x",res);
   else printf("\n! Сигнатура eraseall-патча не найдена");
}   
  
if (bflag) {
   res=patch(patch_erasebad, buf, fsize);
   if (res != 0) printf("\n* Найдена сигнатура isbad по смещению %08x",res);  
   else  printf("\n! Сигнатура isbad не найдена");  
}

if (oflag) {
  out=fopen(outfilename,"w");
  if (out != 0) {
    fwrite(buf,1,fsize,out);
    fclose(out);
  }
  else printf("\n Ошибка открытия выходного файла %s",outfilename);
}
free(buf);
printf("\n");
}

   