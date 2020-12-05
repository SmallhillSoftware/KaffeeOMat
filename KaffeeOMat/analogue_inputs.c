/************************************************************************************
*                                                                                   *
*   File Name   : analogue_inputs.c                                                 *
*   Contents    : Processing concerning analogue inputs for Funkuhr                 *
*   Version     : 1.5                                                               *
*************************************************************************************/ 
#include "globals.h"

//init after reset
void f_vd_adcInit(void)
{
	adcon1 = 0x28; //10bit-mode
	pd0_7         = 0;    /* P0.7 = AN0 is input */
	pd0_6         = 0;    /* P0.6 = AN1 is input */
	pd0_5         = 0;    /* P0.5 = AN2 is input */
	pd0_4         = 0;    /* P0.4 = AN3 is input */
	pu01          = 0;    /* P0.4-7 not pulled-up */
}

void f_vd_AcquireVoltages(void)
{
static unsigned int ui_voltageAcquisitionState;

switch (ui_voltageAcquisitionState)
{
	//undefined
	case d_undefState:
             ui_voltageAcquisitionState = d_startAN2;
             break;
	//start acquisition on AN2
	case d_startAN2:
	     ui_voltageAcquisitionState = d_waitAN2;
             adcon0 = 0x82;  //select CH2
             adst = 1;       //start conversion
             break;
	case d_waitAN2:
	    if (adst == 0)
            {
               ui_voltageAcquisitionState = d_startAN3;
               UI_XDIR_VOLTAGE_AD = ad;
			   B_XDIR_ACCELERATION_ACQUIRED = D_TRUE;
            }
            break;
	//start acquisition on AN3
	case d_startAN3:
	     ui_voltageAcquisitionState = d_waitAN3;
             adcon0 = 0x83;  //select CH3
             adst = 1;       //start conversion
             break;
	case d_waitAN3:
	    if (adst == 0)
            {
               ui_voltageAcquisitionState = d_startAN2;
               UI_VBAT_VOLTAGE_AD = ad;
			   B_VBAT_ACQUIRED = D_TRUE;
            }
            break;	    
} //switch
}
