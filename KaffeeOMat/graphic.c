/***************************************************************************************
*                                                                                      *
*   File Name   : graphic.c                                                            *
*   Contents    : Display driver for Pollin DG14032 adapted to R8C13 (Doppelkopftimer) *
*   Version     : 1.4                                                                  *
****************************************************************************************/ 
#include "globals.h"
#include "graphic.h" 

extern void GLCD_SetPixel(unsigned char x, unsigned char y, unsigned char color);

void GLCD_AlternativeCursor(unsigned char x, unsigned char y)
{
volatile unsigned char x_start_pos = 1, y_pos = 14;
unsigned char x_end_pos;
	x_end_pos   = x_start_pos+6;
	GLCD_ClearGFX();
	//GLCD_Line(x_start_pos,y_pos,x_end_pos,y_pos, 0); //erase old cursor
    x_start_pos = (8*x)+1;
	if (y == 1)
	{
	   y_pos = 30;
	}
	else
	{
	   y_pos = 14;
	}
	x_end_pos   = x_start_pos+6;
	GLCD_Line(x_start_pos,y_pos,x_end_pos,y_pos, 1); //write new cursor
}

void GLCD_Rectangle(unsigned char x, unsigned char y, unsigned char b, unsigned char a, unsigned char color)
{
  unsigned char j;
  for (j = 0; j < a; j++) {
		GLCD_SetPixel(x, y + j, color);
		GLCD_SetPixel(x + b - 1, y + j, color);
	}
  for (j = 0; j < b; j++)	{
		GLCD_SetPixel(x + j, y, color);
		GLCD_SetPixel(x + j, y + a - 1, color);
	}
}
//

void GLCD_Circle(unsigned char cx, unsigned char cy ,unsigned char radius, unsigned char color)
{
	signed int x, y, xchange, ychange, radiusError;
	
	x = radius;
	y = 0;
	xchange = 1 - 2 * radius;
	ychange = 1;
	radiusError = 0;
	while(x >= y) {
  		GLCD_SetPixel(cx+x, cy+y, color); 
  		GLCD_SetPixel(cx-x, cy+y, color); 
  		GLCD_SetPixel(cx-x, cy-y, color);
  		GLCD_SetPixel(cx+x, cy-y, color); 
  		GLCD_SetPixel(cx+y, cy+x, color); 
  		GLCD_SetPixel(cx-y, cy+x, color); 
  		GLCD_SetPixel(cx-y, cy-x, color); 
  		GLCD_SetPixel(cx+y, cy-x, color); 
  		y++;
  		radiusError += ychange;
  		ychange += 2;
  		if ( 2*radiusError + xchange > 0 ) {
    		x--;
			radiusError += xchange;
			xchange += 2;
			}
  		}
}
//

void GLCD_Line(unsigned char X1,unsigned char Y1,unsigned char X2,unsigned char Y2, unsigned char color)
{
	signed int CurrentX, CurrentY, Xinc, Yinc, Dx, Dy, TwoDx, TwoDy, 
		    TwoDxAccumulatedError, TwoDyAccumulatedError;

	Dx = (X2-X1);
	Dy = (Y2-Y1);

	TwoDx = Dx + Dx;
	TwoDy = Dy + Dy;

	CurrentX = X1;
	CurrentY = Y1;

	Xinc = 1;
	Yinc = 1;

	if(Dx < 0)
  		{
  		Xinc = -1;
  		Dx = -Dx;
  		TwoDx = -TwoDx;
  		}

	if (Dy < 0)
  		{
  		Yinc = -1;
  		Dy = -Dy;
  		TwoDy = -TwoDy;
  		}

	GLCD_SetPixel(X1,Y1, color);

	if ((Dx != 0) || (Dy != 0))
  		{
  		if (Dy <= Dx)
    		{ 
    		TwoDxAccumulatedError = 0;
    		do
	  			{
      			CurrentX += Xinc;
      			TwoDxAccumulatedError += TwoDy;
      			if(TwoDxAccumulatedError > Dx)
        			{
        			CurrentY += Yinc;
        			TwoDxAccumulatedError -= TwoDx;
        			}
       			GLCD_SetPixel(CurrentX,CurrentY, color);
       			} while (CurrentX != X2);
     		}
   		else
      		{
      		TwoDyAccumulatedError = 0; 
      		do 
	    		{
        		CurrentY += Yinc; 
        		TwoDyAccumulatedError += TwoDx;
        		if(TwoDyAccumulatedError>Dy) 
          			{
          			CurrentX += Xinc;
          			TwoDyAccumulatedError -= TwoDy;
          			}
         		GLCD_SetPixel(CurrentX,CurrentY, color); 
         		} while (CurrentY != Y2);
    		}
  		}
}

