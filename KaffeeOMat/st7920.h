/***************************************************************************************
*                                                                                      *
*   File Name   : ST7920.h                                                             *
*   Contents    : Display driver for Pollin DG14032 adapted to R8C13 (Doppelkopftimer) *
*   Version     : 1.4                                                                  *
****************************************************************************************/ 
void Delay (unsigned long dval);
void GLCD_ClearTXT(void);
void GLCD_ClearGFX(void);
void GLCD_SetPixel(unsigned char x,unsigned char y, unsigned char color);
void GLCD_VerticalOffset(unsigned char offs);
void GLCD_WriteString(unsigned char x, unsigned char y, char *s);
void GLCD_WriteUINumber(unsigned char x, unsigned char y, unsigned int z, _Bool preventLeadingZeros, unsigned char digitsToDisplay);
void GLCD_WriteULNumber(unsigned char x, unsigned char y, unsigned long z, unsigned char preventLeadingZeros);
void GLCD_Init(void);
void GLCD_SetCursor(unsigned char x, unsigned char y);
void GLCD_CursorOff(void);

