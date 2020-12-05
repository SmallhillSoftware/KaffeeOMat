/************************************************************************************
*                                                                                   *
*   File Name   : MAIN_KAFFEEOMAT.c                                                    *
*   Contents    : Main Loop for KaffeeOMat                                             *
*   Version     : 1.1 bases on main_funkuhr.c 1.17                                                               *
*************************************************************************************/ 

#include "globals.h"
#include "ST7920.h"
#include "graphic.h"
#include "dcf77.h"

extern unsigned long mk_date;

//global variables, changed in ISR
volatile unsigned long UL_TIRQ_count1ms = 0;
volatile unsigned int  UI_STATE = D_undefState;
volatile unsigned int  UI_CLOCK_HOURS = 0;
volatile unsigned int  UI_CLOCK_MINS  = 0;
volatile unsigned int  UI_CLOCK_SECS  = 0;
//global variables
unsigned long UL_int2_IRQ_timestamp = 0;
unsigned int  UI_STATE_CHANGE = 0;
unsigned int  UI_ENC_VALUE;
volatile char C_ENC_DELTA = 0;
_Bool         B_SWITCH_OK = D_FALSE;
unsigned int  UI_CLOCK_YEAR  = 0;
unsigned int  UI_CLOCK_MONTH = 0;
unsigned int  UI_CLOCK_DAY   = 0;

void main (void)
{
unsigned char uc_valid_chk_result;
    //start initialization of the uC
    f_vd_controllerInit();
    GLCD_Init();
    GLCD_ClearTXT();
    GLCD_ClearGFX();

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
					UI_STATE = D_adjustTimeState;
					UI_STATE_CHANGE = 1;
				} //end of if (UL_TIRQ_count1ms >= D_SHOW_REV_TIME_MS)
				break; //of case D_revDisplayState:
			case D_adjustTimeState:
				if (UI_STATE_CHANGE)
				{
					GLCD_WriteString(0,0,"Zeit:           ");
					GLCD_WriteString(0,1,"DCF77-Sync:     ");
					GLCD_WriteString(7,0,".");
					GLCD_WriteString(10,0,"-");
					GLCD_WriteString(13,0,":");
					UI_STATE_CHANGE = 0;
				} //end of if (UI_STATE_CHANGE)
				GLCD_WriteUINumber(5,0,UI_CLOCK_DAY,1,2);				
				GLCD_WriteUINumber(8,0,UI_CLOCK_MONTH,1,2);
				GLCD_WriteUINumber(11,0,UI_CLOCK_HOURS,1,2);
				GLCD_WriteUINumber(14,0,UI_CLOCK_MINS ,1,2);
				GLCD_WriteUINumber(14,1,UI_CLOCK_SECS ,1,2);
				GLCD_WriteUINumber(11,1,f_uc_get_dcf77_state(),1,1);
				if (f_uc_get_dcf77_state() == D_dcf77_state_FRAME_RECEIVED)
				{
					uc_valid_chk_result = f_uc_check_dcf77_frame_validity(UI_CLOCK_SECS, UI_CLOCK_MINS, UI_CLOCK_HOURS);
					if (f_uc_get_dcf77_state() == D_dcf77_state_FRAME_CHECKED)
					{
						//t2>3
						uc_valid_chk_result = f_uc_check_dcf77_date_validity();
						if (f_uc_get_dcf77_state() == D_dcf77_state_DATE_VALID)
						{
							UI_CLOCK_SECS  = 0;
							UI_CLOCK_HOURS = (unsigned int)(f_uc_dcf77_get_date_hour());
							UI_CLOCK_MINS  = (unsigned int)(f_uc_dcf77_get_date_minute());
							#ifdef DATE_REQUIRED
								UI_CLOCK_DAY   = (unsigned int)(f_uc_dcf77_get_date_day());
								UI_CLOCK_MONTH = (unsigned int)(f_uc_dcf77_get_date_month());
								UI_CLOCK_YEAR  = f_ui_dcf77_get_date_year();
							#endif
							f_vd_DCF77_disableIRQ();
							UI_STATE = D_runningState;
							UI_STATE_CHANGE = 1;
						} //end of if (f_uc_get_dcf77_state() == D_dcf77_state_DATE_VALID)
					} //end of if (f_uc_get_dcf77_state() == D_dcf77_state_FRAME_CHECKED)
				} //end of if (UC_DCF77_STATE == D_dcf77_state_FRAME_RECEIVED)
				//t8
				if (B_SWITCH_OK == D_TRUE)
				{  
					B_SWITCH_OK = D_FALSE;    
					UI_STATE = D_runningState;
				}
				break; //of case D_adjustTimeState
			case D_runningState:
				if (UI_STATE_CHANGE)
				{
					GLCD_WriteString(0,0,"Zeit:           ");
					GLCD_WriteString(0,1,"Kaff:           ");
					GLCD_WriteString(13,0,":");
					GLCD_WriteString(13,1,":");
					GLCD_WriteString(7,0,".");
					GLCD_WriteString(7,1,".");
					UI_STATE_CHANGE = 0;
				} //end of if (UI_STATE_CHANGE)
				GLCD_WriteUINumber(11,0,UI_CLOCK_HOURS,1,2);
				GLCD_WriteUINumber(14,0,UI_CLOCK_MINS ,1,2);
				GLCD_WriteUINumber(8,0,UI_CLOCK_MONTH,1,2);
				GLCD_WriteUINumber(5,0,UI_CLOCK_DAY,1,2);
				//t3>2
				if ( (UI_CLOCK_HOURS == 3) &&
				     (UI_CLOCK_MINS == 15)
				   )
				{
					/* at 3:15 a new DCF77-reception will be triggered */					
					f_vd_DCF77_init(D_TRUE);
					UI_STATE = D_adjustTimeState;
					UI_STATE_CHANGE = 1;
				}				
				break; //of case D_runningState
		} //end of switch (UI_STATE) 
	} //end of while(1)
} //end of void main (void)
