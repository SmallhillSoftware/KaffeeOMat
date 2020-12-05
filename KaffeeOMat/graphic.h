/***************************************************************************************
*                                                                                      *
*   File Name   : graphic.h                                                            *
*   Contents    : Display driver for Pollin DG14032 adapted to R8C13 (Doppelkopftimer) *
*   Version     : 1.2                                                                  *
****************************************************************************************/ 
void GLCD_AlternativeCursor(unsigned char x, unsigned char y);
void GLCD_Rectangle(unsigned char x, unsigned char y, unsigned char b, unsigned char a, unsigned char color);
void GLCD_Circle(unsigned char cx, unsigned char cy ,unsigned char radius, unsigned char color);
void GLCD_Line(unsigned char X1,unsigned char Y1,unsigned char X2,unsigned char Y2, unsigned char color);
