/************************************************************************************
*                                                                                   *
*   File Name   : globals.c                                                         *
*   Contents    : Global Support Functions for KaffeeOMat                           *
*   Version     : 1.10, bases on 1.6 from Funkuhr 20201205 and 1.2 from MatchDisplay 20211026                           *
*************************************************************************************/ 
#include "globals.h"
#include "dcf77.h"



//init after reset
void f_vd_controllerInit(void)
{
    asm("FCLR I");                          /* Interrupt disable */

    /*-----------------------------------------------
    -Change on-chip oscillator clock to Main clock  -
    -------------------------------------------------*/
    prc0 = 1;                               /* Protect off */
    cm13 = 1;                               /* Xin Xout */
    cm15 = 1;                               /* XCIN-XCOUT drive capacity select bit : HIGH */
    cm05 = 0;                               /* Xin on */
    cm16 = 0;                               /* Main clock = No division mode */
    cm17 = 0;
    cm06 = 0;                               /* CM16 and CM17 enable */
    asm("nop");                             /* Waiting for stable of oscillation */
    asm("nop");
    asm("nop");
    asm("nop");
    ocd2 = 0;                               /* Main clock change */
    prc0 = 0;                               /* Protect on */

    /*-----------------------------------
    -   Initialize SFR                  -
    -----------------------------------*/
    
    f_vd_TIMERX_init();               /* Initial setting of TIMER X registers */

    f_vd_TIMERY_init();               /* Initial setting of TIMER Y registers */

    f_vd_TIMERZ_init();               /* Initial setting of TIMER Z registers */

    f_vd_EXTINTS_init();              /* Initial setting of EXTINT registers  */

    f_vd_DCF77_init(D_TRUE);               /* Initial setting of DCF77-environment with hard reset */

    f_vd_adcInit();

    #ifdef UART1_DEBUGGING
        f_vd_UART1_init();
    #endif

    //settings for LED
    ActLED_port_dir = 1; //Output
    
    //settings for backlight duty operation
	 BacklightMod_port_dir = 1; //port is output    
    BacklightMod_port = 0;     //output is off
    UC_BACKLIGHT_DUTY = 0;     //30% duty cycle but shall be off
    
    asm("FSET I");                            /* Interrupt enable */

}


