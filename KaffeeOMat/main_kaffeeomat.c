/************************************************************************************
*                                                                                   *
*   File Name   : MAIN_KAFFEEOMAT.c                                                 *
*   Contents    : Main Loop for KaffeeOMat                                          *
*   Version     : 1.9 completely new approach                                       *
*************************************************************************************/ 

#include "globals.h"
#include "ST7920.h"
#include "graphic.h"

extern unsigned long mk_date;

//global variables, changed in ISR
volatile unsigned long UL_TIRQ_count1ms = 0;
volatile unsigned char UC_TIRQ_pwmCount = 0;
volatile unsigned int  UI_STATE = D_undefState;
volatile unsigned int  UI_CLOCK_HOURS = 0;
volatile unsigned int  UI_CLOCK_MINS  = 0;
volatile unsigned int  UI_CLOCK_SECS  = 0;
volatile unsigned int  UI_CLOCK_DAYS  = 0;
//global variables
unsigned long UL_int2_IRQ_timestamp = 0;
unsigned int  UI_STATE_CHANGE = 0;
unsigned int  UI_ENC_VALUE;
volatile char C_ENC_DELTA = 0;
_Bool         B_SWITCH_OK = D_FALSE;
volatile unsigned int  UI_VBAT_VOLTAGE_MV = 0;
_Bool         B_VBAT_ACQUIRED = D_FALSE;
_Bool         B_BACKLIGHT_ON = D_FALSE;
volatile unsigned char UC_BACKLIGHT_DUTY = 0;

unsigned long UL_START_TIMESTAMP = 0;

void main (void)
{
    //start initialization of the uC
    f_vd_controllerInit();
    GLCD_Init();
    GLCD_ClearTXT();
    GLCD_ClearGFX();
    B_BACKLIGHT_ON = D_TRUE;

    /*-----------------------------------
    -   Loop of main                    -
    -----------------------------------*/
	while(1)
	{
		//Calculations
      //encoder input to minutes to run conversion
		if (C_ENC_DELTA < 0)
		{
			UI_ENC_VALUE =  (-1 * C_ENC_DELTA)/2;
		} //end of if (C_ENC_DELTA < 0)
		else
		{
			UI_ENC_VALUE =  C_ENC_DELTA/2;
		} //end of else of if (C_ENC_DELTA < 0)
		if (UI_ENC_VALUE > 99)
		{
			UI_ENC_VALUE = 99;
		} //end of if (UI_ENC_VALUE > 99)
        
		//Statemachine
		switch (UI_STATE)
		{
			case D_undefState:
				//t0>1
				UI_STATE = D_revDisplayState;
				UI_STATE_CHANGE = 1;
				break; //of case D_undefState
			case D_revDisplayState:
				if (UI_STATE_CHANGE)
				{
					GLCD_WriteULNumber(8,0,mk_date,1);					
					GLCD_WriteString(0,0,"kaffeeOmat");
					GLCD_WriteString(0,1,"www.smallhill.de");
					GLCD_Rectangle(0,0,140,31,1);	
					UI_STATE_CHANGE = 0;
				} //end of if (UI_STATE_CHANGE)
				//t1>2
				if (UL_TIRQ_count1ms >= D_SHOW_REV_TIME_MS) //leave showRevState after a defined amount of time
				{
					UI_STATE = D_runningState;
					UI_STATE_CHANGE = 1;
				} //end of if (UL_TIRQ_count1ms >= D_SHOW_REV_TIME_MS)
				break; //of case D_revDisplayState:
			case D_runningState:
				if (UI_STATE_CHANGE)
				{
					GLCD_WriteString(0,0,"Der Kaffee ist  ");
					GLCD_WriteString(0,1,"XXtYYhZZmQQs alt");
					UI_STATE_CHANGE = 0;
				} //end of if (UI_STATE_CHANGE)
				GLCD_WriteUINumber(9,1,UI_CLOCK_SECS,1,2);
				GLCD_WriteUINumber(6,1,UI_CLOCK_MINS ,1,2);
				GLCD_WriteUINumber(3,1,UI_CLOCK_HOURS,1,2);
				GLCD_WriteUINumber(0,1,UI_CLOCK_DAYS,1,2);
				//t8
				if (B_SWITCH_OK == D_TRUE)
				{  
					B_SWITCH_OK = D_FALSE;    
					UI_CLOCK_SECS = 0;
					UI_CLOCK_MINS = 0;
					UI_CLOCK_HOURS = 0;
					UI_CLOCK_DAYS = 0;
				}
				if (UI_VBAT_VOLTAGE_MV < D_WARNLEVEL_VBAT_MV)
				{
					UI_STATE = D_warnVbattState;
					UI_STATE_CHANGE = 1;
				}				
				break; //of case D_runningState
			case D_warnVbattState:
				if (UI_STATE_CHANGE)
				{
					UL_START_TIMESTAMP = UL_TIRQ_count1ms;
					GLCD_WriteString(0,0,"Batteriestand   ");
					GLCD_WriteString(0,1,"niedrig:      mV");
					UI_STATE_CHANGE = 0;
				} //end of if (UI_STATE_CHANGE)
				GLCD_WriteUINumber(9,1,UI_VBAT_VOLTAGE_MV,1,4);
				//t8
				if ((UL_TIRQ_count1ms - UL_START_TIMESTAMP) > D_SHOW_REV_TIME_MS)
				{  
					UI_STATE = D_runningState;
					UI_STATE_CHANGE = 1;
				}				
				break; //of case D_setKaffeeTime
		} //end of switch (UI_STATE) 
	} //end of while(1)
} //end of void main (void)
