
// Структура, описывающая сигнатуру и положение патча
struct defpatch {
 const char* sig; // сигнатрура
 uint32_t sigsize; // длина сигнатуры
 int32_t poffset;  // смещение до точки патча от конца сигнатуры
};



//***********************************************************************
//* Поиск сигнатуры и наложение патча
//***********************************************************************
uint32_t patch(struct defpatch fp, uint8_t* buf, uint32_t fsize, uint32_t ptype);

//****************************************************
//* Процедуры патча под разные чипсеты и задачи
//****************************************************

uint32_t pv7r22 (uint8_t* buf, uint32_t fsize);
uint32_t pv7r22_2 (uint8_t* buf, uint32_t fsize);
uint32_t pv7r2 (uint8_t* buf, uint32_t fsize);
uint32_t pv7r11 (uint8_t* buf, uint32_t fsize);
uint32_t pv7r1 (uint8_t* buf, uint32_t fsize);
uint32_t perasebad (uint8_t* buf, uint32_t fsize);

