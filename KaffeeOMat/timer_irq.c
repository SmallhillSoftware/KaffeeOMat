/************************************************************************************
*                                                                                   *
*   File Name   : timer_irq.c                                                       *
*   Contents    : Timer interrupt related functions of KaffeeOMat                   *
*   Version     : 1.4, basierend auf 1.2 vom Projekt Funkuhr 20201205               *
*************************************************************************************/ 
#include "globals.h"

/************************************************************************************
Name:        TIMERX_init
Parameters:  None
Returns:     None
Description: Initial setting of Timer X registers
************************************************************************************/
void f_vd_TIMERX_init(void)
{
    txck0 = 1;                              /* Timer X count source = f8 */
    txck1 = 0;

    /* Setting the main cycle timer                   */
    /* 20 MHz * 1/8 * 1/50 * 1/10 => 250us  */
    prex = 50-1;                            /* Setting Prescaler X register */
    tx   = 10-1;                            /* Setting timer X register */

	
    txmr = 0x00;                            /* Timer X : timer mode */

    txs  = 0;                               /* Timer X count start flag = stop */
}
/************************************************************************************
Name:        TIMERY_init
Parameters:  None
Returns:     None
Description: Initial setting of Timer Y registers
************************************************************************************/
void f_vd_TIMERY_init(void)
{
    tyck0 = 1;                              /* Timer Y count source = f8 */
    tyck1 = 0;

    /* Setting the main cycle timer                   */
    /* 20 MHz * 1/8 * 1/20 * 1/125 => 100 Hz = 1ms  */
    prey   = 20-1;                          /* Setting Prescaler Y register */
    tysc   = 0;                             /* Setting timer Y secondary register */
    typr   = 125-1;                         /* Setting timer Y register */

    tymod0 = 0;                             /* Timer Y : timer mode */
    tywc   = 0;                             /* Write to timer and reload register simultaneously */

    tyic = 5;                               /* Interrupt priority level = 5 */

    ir_tyic = 0;                            /* Interrupt request flag clear */
    tys  = 1;                               /* Timer Y count start flag = start */
}
/************************************************************************************
Name:        TIMERZ_init
Parameters:  None
Returns:     None
Description: Initial setting of Timer Y registers
************************************************************************************/
void f_vd_TIMERZ_init(void)
{
    tzs   = 0;                              /* Timer Z = stopped */
    tzck0 = 0;                              /* Timer Y underflow @1ms = count source */
    tzck1 = 1;

    prez   = 4-1;                           /* Setting Prescaler Z register */
    tzsc   = 200-1;                         /* Setting timer Z secondary register */
    tzpr   = 50-1;                          /* Setting timer Z primary register */

    tzmod0 = 1;                             
    tzmod1 = 0;                             /* Timer Z : Programmable waveform generation mode */
    tzwc   = 1;                             /* Write to timer and reload register simultaneously ? */

    tzocnt = 0;                             /* TZ-Output triggered from timer not GPIO */
    tzopl  = 0;                             /* TZ-Output H for primary, L for secondary, L for timer stop */
    inostg = 0;                             /* no interrupt trigger */    
    inoseg = 0;                             /* interrupt polarity */

}

/************************************************************************************
Name:        interrupt Timer Y
Parameters:  None
Returns:     None
Description: Timer Y
************************************************************************************/
#pragma interrupt timer_1ms_irq
void timer_1ms_irq(void)
{
unsigned long ul_inc_page_period = 0;
	//disable all IRQ's
	asm("FCLR I");
	UL_TIRQ_count1ms++;                           /* 1 msecond counter increment */
	UC_TIRQ_pwmCount++;

	//read encoder every 1ms
	if (UI_STATE == D_adjustTimeState)
	{
	   f_vd_ENCODER_READ();
	}
	
	//PWM-Output for LCD-Backlight, 100ms base frequency
	if ( (UL_TIRQ_count1ms%100) == 0 )
	{
		UC_TIRQ_pwmCount = 0;
		BacklightMod_port = 1; //switch port on
		f_vd_AcquireVoltages();
		if (B_VBAT_ACQUIRED == D_TRUE)
		{
			UC_BACKLIGHT_DUTY = D_CONV_VBAT_MV_2_DC(UI_VBAT_VOLTAGE_MV);
			B_VBAT_ACQUIRED = D_FALSE;
		} //end of if (B_VBAT_ACQUIRED == D_TRUE)
	} //end of if ( (UL_TIRQ_count1ms%100) == 0 )
	if (UC_TIRQ_pwmCount == UC_BACKLIGHT_DUTY)
	{
		BacklightMod_port = 0; //switch port off
	}

	//toggle LED all 1000ms
	if ( (UL_TIRQ_count1ms%1000) == 0 )           /* 1 second have passed ? */
    	{
		if (UI_CLOCK_SECS == 59)
		{
			UI_CLOCK_SECS = 0;
			if (UI_CLOCK_MINS == 59)
			{
				UI_CLOCK_MINS = 0;
				if (UI_CLOCK_HOURS == 23)
				{
					UI_CLOCK_HOURS = 0;
				}
				else
				{
					UI_CLOCK_HOURS++;
				}
			}
			else
			{
				UI_CLOCK_MINS++;
			}
		}
		else
		{
			UI_CLOCK_SECS++;
		}
        	ActLED_port = ~ActLED_port;
                #ifdef UART1_DEBUGGING
		    f_vd_sendValues();                    /* send XDIR, YDIR, ZDIR, POS values on UART1 for debugging purposes */
                #endif
	}
	//enable all IRQ's
	asm("FSET I");
}
