/************************************************************************************
*                                                                                   *
*   File Name   : analogue_inputs.c                                                 *
*   Contents    : Processing concerning analogue inputs for KaffeeOMat              *
*   Version     : 1.2                                                               *
*************************************************************************************/ 
#include "globals.h"

//init after reset
void f_vd_adcInit(void)
{
	adcon1        = 0x28;   /* bits = 1 (10bit); vcut = 1 (Vref connected); cks1 = 0 */
	//pd0_7         = 0;    /* P0.7 = AN0 is input  */
	//pd0_6         = 0;    /* P0.6 = AN1 is input  */
	//pd0_5         = 0;    /* P0.5 = AN2 is input  */
	//pd0_4         = 0;    /* P0.4 = AN3 is input  */
	//pu01          = 0;    /* P0.4-7 not pulled-up */
	pu02          = 0;    /* P1.0-3 not pulled-up */
	pd1_3         = 0;    /* P1.3 is input */
	ki3en         = 0;    /* KEY-In 3 is not enabled */
}

void f_vd_AcquireVoltages(void)
{
static unsigned int ui_voltageAcquisitionState;

	switch (ui_voltageAcquisitionState)
	{
		//undefined
		case d_undefState:
			ui_voltageAcquisitionState = d_startAN11;
			break;
		//start acquisition on AN11
		case d_startAN11:
			if (B_VBAT_ACQUIRED == D_FALSE)
			{
				ui_voltageAcquisitionState = d_waitAN11;
				adcon0 = 0x97;  /*cks0 = 1; adst = 0; adgsel0 = 1; md = 0; select CH11 = 111 */
				adst = 1;       //start conversion
			}
			break;
		case d_waitAN11:
			if (adst == 0)
			{
				ui_voltageAcquisitionState = d_startAN11;
				UI_VBAT_VOLTAGE_MV = D_CONV_ADC_2_VBAT_MV(ad);
				B_VBAT_ACQUIRED = D_TRUE;
			}
			break;
	} //end of switch (ui_voltageAcquisitionState)
}
