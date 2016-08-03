// Загрузчик usbloader.bin через аварийный порт для модемов на платформе Balong V7R2.
//
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <termios.h>
//#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include <arpa/inet.h>
#ifndef WIN32
#include <termios.h>
#include <unistd.h>
#include <arpa/inet.h>
#else
#include <windows.h>
#include "wingetopt.h"
#include "printf.h"
#endif


#ifndef WIN32
int siofd;
struct termios sioparm;
#else
static HANDLE hSerial;
#endif
FILE* ldr;


//*************************************************
//* HEX-дамп области памяти                       *
//*************************************************

void dump(unsigned char buffer[],int len) {
int i,j;
unsigned char ch;

printf("\n");
for (i=0;i<len;i+=16) {
  printf("%04x: ",i);
  for (j=0;j<16;j++){
   if ((i+j) < len) printf("%02x ",buffer[i+j]&0xff);
   else printf("   ");}
  printf(" *");
  for (j=0;j<16;j++) {
   if ((i+j) < len) {
    // преобразование байта для символьного отображения
    ch=buffer[i+j];
    if ((ch < 0x20)||((ch > 0x7e)&&(ch<0xc0))) putchar('.');
    else putchar(ch);
   } 
   // заполнение пробелами для неполных строк 
   else printf(" ");
  }
  printf("*\n");
 }
}


//*************************************************
//* Рассчет контрольной суммы командного пакета
//*************************************************
void csum(unsigned char* buf, int len) {

unsigned  int i,c,csum=0;

unsigned int cconst[]={0,0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF};

for (i=0;i<(len-2);i++) {
  c=(buf[i]&0xff);
  csum=((csum<<4)&0xffff)^cconst[(c>>4)^(csum>>12)];
  csum=((csum<<4)&0xffff)^cconst[(c&0xf)^(csum>>12)];
}  
buf[len-2]=(csum>>8)&0xff;
buf[len-1]=csum&0xff;
  
}

//*************************************************
//*   Отсылка командного пакета модему
//*************************************************
int sendcmd(unsigned char* cmdbuf, int len) {

unsigned char replybuf[1024];
unsigned int replylen;

#ifndef WIN32
csum(cmdbuf,len);
write(siofd,cmdbuf,len);  // отсылка команды
tcdrain(siofd);
replylen=read(siofd,replybuf,1024);
#else
    DWORD bytes_written = 0;
    DWORD t;

    csum(cmdbuf, len);
    WriteFile(hSerial, cmdbuf, len, &bytes_written, NULL);
    FlushFileBuffers(hSerial);

    t = GetTickCount();
    do {
        ReadFile(hSerial, replybuf, 1024, (LPDWORD)&replylen, NULL);
    } while (replylen == 0 && GetTickCount() - t < 1000);
#endif
if (replylen == 0) return 0;    
if (replybuf[0] == 0xaa) return 1;
return 0;
}






//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

void main(int argc, char* argv[]) {

#ifndef WIN32
struct termios sioparm;
#else
    char device[20] = "\\\\.\\COM";
    DCB dcbSerialParams = {0};
    COMMTIMEOUTS CommTimeouts;
#endif
//char* lptr;
unsigned int i,res,opt,datasize,pktcount,adr;
int nblk;   // число блоков для загрузки
int bl;    // текущий блок

unsigned char cmdhead[14]={0xfe,0, 0xff};
unsigned char cmddata[1040]={0xda,0,0};
unsigned char cmdeod[5]={0xed,0,0,0,0};

struct {
  int lmode;  // режим загрузки: 1 - прямой старт, 2 - через перезапуск A-core
  int size;   // размер блока
  int adr;    // адрес загрузки блока в память
  int offset; // смещение до блока от начала файла
} blk[10];

#ifndef WIN32
unsigned char devname[50]="/dev/ttyUSB0";
#else
char devname[50]="";
#endif

while ((opt = getopt(argc, argv, "hp:sa:")) != -1) {
  switch (opt) {
   case 'h': 
     
printf("\n Утилита предназначена для аварийной USB-загрузки модемов E3372S\n\n\
%s [ключи] <имя файла для загрузки>\n\n\
 Допустимы следующие ключи:\n\n\
-p <tty> - последовательный порт для общения с загрузчиком (по умолчанию /dev/ttyUSB0\n\
\n",argv[0]);
    return;

   case 'p':
    strcpy(devname,optarg);
    break;
        
  }
}  

if (optind>=argc) {
    printf("\n - Не указано имя файла для загрузки\n");
    return;
}  

#ifdef WIN32
if (*devname == '\0')
{
   printf("\n - Последовательный порт не задан\n"); 
   return; 
}
#endif

#ifndef WIN32
// Настройка SIO
siofd = open(devname, O_RDWR | O_NOCTTY |O_SYNC);
if (siofd == -1) {
    printf("\n - Последовательный порт %s не открывается\n", argv[1]);
    return;
 }

bzero(&sioparm, sizeof(sioparm));
sioparm.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
sioparm.c_iflag = 0;  // INPCK;
sioparm.c_oflag = 0;
sioparm.c_lflag = 0;
sioparm.c_cc[VTIME]=1;  // таймаут 
sioparm.c_cc[VMIN]=1;   // 1 байт минимального ответа

tcsetattr(siofd, TCSANOW, &sioparm);
tcflush(siofd,TCIOFLUSH);  // очистка выходного буфера
#else
    strcat(device, /*(char*)*/devname);
    
    hSerial = CreateFileA(device, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        printf("\n - Последовательный порт COM%s не открывается\n", devname); 
        return;
    }

    ZeroMemory(&dcbSerialParams, sizeof(dcbSerialParams));
    dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
    dcbSerialParams.BaudRate = CBR_115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    dcbSerialParams.fBinary = TRUE;
    dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;
    dcbSerialParams.fRtsControl = RTS_CONTROL_ENABLE;
    if (!SetCommState(hSerial, &dcbSerialParams))
    {
        printf("\n - Ошибка при инициализации COM-порта\n", devname); 
        CloseHandle(hSerial);
        return;
    }

    CommTimeouts.ReadIntervalTimeout = MAXDWORD;
    CommTimeouts.ReadTotalTimeoutConstant = 0;
    CommTimeouts.ReadTotalTimeoutMultiplier = 0;
    CommTimeouts.WriteTotalTimeoutConstant = 0;
    CommTimeouts.WriteTotalTimeoutMultiplier = 0;
    if (!SetCommTimeouts(hSerial, &CommTimeouts))
    {
        printf("\n - Ошибка при инициализации COM-порта\n", devname); 
        CloseHandle(hSerial);
        return;
    }
#endif

ldr=fopen(argv[optind],"rb");
if (ldr == 0) {
  printf("\n Ошибка открытия %s",argv[optind]);
  return;
}

// Прверяем сигнатуру usloader
fread(&i,1,4,ldr);
if (i != 0x20000) {
  printf("\n Файл %s не является загрузчиком usbloader\n",argv[optind]);
  return;
}  

fseek(ldr,36,SEEK_SET); // начало описателей блоков для загрузки

// Разбираем заголовок

for(nblk=0;nblk<10;nblk++) {
 fread(&blk[nblk].lmode,1,4,ldr);
 fread(&blk[nblk].size,1,4,ldr);
 fread(&blk[nblk].adr,1,4,ldr);
 fread(&blk[nblk].offset,1,4,ldr);
 if (blk[nblk].lmode == 0) break;
} 
printf("\n Найдено %i блоков для загрузки",nblk);

// главный цикл загрузки - загружаем все блоки, найденные в заголовке

for(bl=0;bl<nblk;bl++) {
  
  printf("\n Загрузка блока %i, адрес=%08x, размер=%i\n",bl,blk[bl].adr,blk[bl].size);
  fseek(ldr,blk[bl].offset,SEEK_SET);
  // фрмируем пакет начала блока  
  *((unsigned int*)&cmdhead[4])=htonl(blk[bl].size);
  *((unsigned int*)&cmdhead[8])=htonl(blk[bl].adr);
  cmdhead[3]=blk[bl].lmode;
  
  // отправляем пакет начала блока
  res=sendcmd(cmdhead,14);
  if (!res) {
    printf("\nМодем отверг пакет заголовка\nОбраз пакета:");
    dump(cmdhead,14);
    return;
  }  

  // Цикл поблочной загрузки данных
  datasize=1024;
  pktcount=1;

  for(adr=0;adr<blk[bl].size;adr+=1024) {
    if ((adr+1024)>=blk[bl].size) datasize=blk[bl].size-adr;  
#ifndef WIN32
    fprintf(stderr,"\r Адрес: %08x, пакет# %i  размер: %i",blk[bl].adr+adr,pktcount,datasize);
#else
    printf("\r Адрес: %08x, пакет# %i  размер: %i",blk[bl].adr+adr,pktcount,datasize);
#endif

    // читаем порцию данных
    if (datasize != fread(cmddata+3,1,datasize,ldr)) {
      printf("\n Неожиданный конец файла\n");
      return;
    }
  
    // готовим пакет данных
    cmddata[1]=pktcount;
    cmddata[2]=(~pktcount)&0xff;
    pktcount++;
    if (!sendcmd(cmddata,datasize+5)) {
      printf("\nМодем отверг пакет данных\nОбраз пакета:");
      dump(cmddata,datasize+5);
      return;
    }  
  }

  // Фрмируем пакет конца данных
  cmdeod[1]=pktcount;
  cmdeod[2]=(~pktcount)&0xff;

  if (!sendcmd(cmdeod,5)) {
    printf("\nМодем отверг пакет конца данных\nОбраз пакета:");
    dump(cmdeod,5);
  }  
} 
printf("\n Загрузка окончена\n");  
}


