/************************************************************************************
*                                                                                   *
*   File Name   : uart_functions.c                                                  *
*   Contents    : UART-related functions for the Funkuhr                            *
*   Version     : 1.7                                                               *
*************************************************************************************/ 
#include "globals.h"
#include "dcf77.h"

#ifdef UART1_DEBUGGING

//#define BIT_TIMINGS
//#define RECEIVED_BITS
//#define FRAME_VALIDITY

/************************************************************************************
Name:        sendTxd1
Parameters:  unsigned char data - byte to be send via UART1
Returns:     None
Description: Initial setting of UART1 registers
************************************************************************************/
void f_vd_sendTxd1(unsigned char data)
{
	while (ti_u1c1 == 0);
	u1tbl = data;
	te_u1c1 = 1;
}

/************************************************************************************
Name:        UART1_init
Parameters:  None
Returns:     None
Description: Initial setting of UART1 registers
************************************************************************************/
void f_vd_UART1_init(void)
{
        pd0_0 = 1;                              /* P0.0 = TxD11 Output */
        pd3_7 = 0;                              /* P3.7 = RxD1 Input */
	u1mr = 0x05; /* 8 bits to transmit */
	u1c0 = 0x00;
	u0rrm = 0;
	u1brg = 130-1;
	rxd1en = 0;
	txd1sel = 1;
	te_u1c1 = 1;
}

/************************************************************************************
Name:        sendValues
Parameters:  None
Returns:     None
Description: send values on UART1
************************************************************************************/
void f_vd_sendValues(void)
{
unsigned long value;
unsigned char digit;
static _Bool B_page0_1 = 1;

f_vd_sendTxd1('s'); /* send start character 's' */

#ifdef BIT_TIMINGS
	//####################################
	f_vd_sendTxd1('D');
	f_vd_sendTxd1('U');
	f_vd_sendTxd1('R');
	f_vd_sendTxd1(':');
	digit = UL_DCF77_BIT_DURATION/1000000000;
	f_vd_sendTxd1(0x30+digit);
	value = UL_DCF77_BIT_DURATION - (digit*1000000000);
	digit = value/100000000;
	f_vd_sendTxd1(0x30+digit);
	value = value - (digit*100000000);
	digit = value/10000000;
	f_vd_sendTxd1(0x30+digit);
	value = value - (digit*10000000);
	digit = value/1000000;
	f_vd_sendTxd1(0x30+digit);
	value = value - (digit*1000000);
	digit = value/100000;
	f_vd_sendTxd1(0x30+digit);
	value = value - (digit*100000);
	digit = value/10000;
	f_vd_sendTxd1(0x30+digit);
	value = value - (digit*10000);
	digit = value/1000;
	f_vd_sendTxd1(0x30+digit);
	value = value - (digit*1000);
	digit = value/100;
	f_vd_sendTxd1(0x30+digit);
	value = value - (digit*100);
	digit = value/10;
	f_vd_sendTxd1(0x30+digit);
	value = value - (digit*10);
	f_vd_sendTxd1(0x30+value);
	//####################################
	f_vd_sendTxd1('D');
	f_vd_sendTxd1('I');
	f_vd_sendTxd1('S');
	f_vd_sendTxd1(':');
	digit = UL_DCF77_BIT_DISTANCE/1000000000;
	f_vd_sendTxd1(0x30+digit);
	value = UL_DCF77_BIT_DISTANCE - (digit*1000000000);
	digit = value/100000000;
	f_vd_sendTxd1(0x30+digit);
	value = value - (digit*100000000);
	digit = value/10000000;
	f_vd_sendTxd1(0x30+digit);
	value = value - (digit*10000000);
	digit = value/1000000;
	f_vd_sendTxd1(0x30+digit);
	value = value - (digit*1000000);
	digit = value/100000;
	f_vd_sendTxd1(0x30+digit);
	value = value - (digit*100000);
	digit = value/10000;
	f_vd_sendTxd1(0x30+digit);
	value = value - (digit*10000);
	digit = value/1000;
	f_vd_sendTxd1(0x30+digit);
	value = value - (digit*1000);
	digit = value/100;
	f_vd_sendTxd1(0x30+digit);
	value = value - (digit*100);
	digit = value/10;
	f_vd_sendTxd1(0x30+digit);
	value = value - (digit*10);
	f_vd_sendTxd1(0x30+value);
#endif //BIT_TIMINGS

#ifdef RECEIVED_BITS
if (UC_DCF77_FIELD_CNT > 0)
{
	f_vd_sendTxd1('C');
	f_vd_sendTxd1('N');
	f_vd_sendTxd1('T');
	f_vd_sendTxd1(':');
	digit = (UC_DCF77_FIELD_CNT-1)/10;
	f_vd_sendTxd1(0x30+digit);
	value = (UC_DCF77_FIELD_CNT-1) - (digit*10);
	f_vd_sendTxd1(0x30+value);
	//####################################
	f_vd_sendTxd1('V');
	f_vd_sendTxd1('A');
	f_vd_sendTxd1('L');
	f_vd_sendTxd1(':');
	value = UC_DCF77_BIT_FIELD[UC_DCF77_FIELD_CNT-1];
	f_vd_sendTxd1(0x30+value);
	//####################################
}
#endif // RECEIVED_BITS

#ifdef FRAME_VALIDITY
	if (B_page0_1 == 0)
	{
		B_page0_1 = 1;
	}
	else
	{
		B_page0_1 = 0;
	}
	if (f_uc_get_dcf77_state() == D_dcf77_state_SYNC_FOUND)
	{
		if (UC_DCF77_FIELD_CNT > 0)
		{	
			f_vd_sendTxd1('C');
			f_vd_sendTxd1('N');
			f_vd_sendTxd1('T');
			f_vd_sendTxd1(':');
			digit = (UC_DCF77_FIELD_CNT-1)/10;
			f_vd_sendTxd1(0x30+digit);
			value = (UC_DCF77_FIELD_CNT-1) - (digit*10);
			f_vd_sendTxd1(0x30+value);
			//####################################
			f_vd_sendTxd1('V');
			f_vd_sendTxd1('A');
			f_vd_sendTxd1('L');
			f_vd_sendTxd1(':');
			value = UC_DCF77_BIT_FIELD[UC_DCF77_FIELD_CNT-1];
			f_vd_sendTxd1(0x30+value);
			//####################################
		} //end of if (UC_DCF77_FIELD_CNT > 0)
	} //end of if (f_uc_get_dcf77_state() == D_dcf77_state_SYNC_FOUND)
	else
	{
		if ((ST_MINUTE.uc_noOfTmpDateVals > 0) && !(B_page0_1))
		{
			if (ST_MINUTE.uc_noOfTmpDateVals == 1)
			{
				f_vd_sendTxd1('D');
				f_vd_sendTxd1('F');
				f_vd_sendTxd1('0');
				f_vd_sendTxd1(':');
				digit = ST_MINUTE.ul_tmpDV_timestamp[0]/1000;
				f_vd_sendTxd1(0x30+digit);
				value = ST_MINUTE.ul_tmpDV_timestamp[0] - (digit*1000);
				digit = value/100;
				f_vd_sendTxd1(0x30+digit);
				value = value - (digit*100);
				digit = value/10;
				f_vd_sendTxd1(0x30+digit);
				value = value - (digit*10);
				f_vd_sendTxd1(0x30+value);				
				//####################################
				f_vd_sendTxd1('M');
				f_vd_sendTxd1('N');
				f_vd_sendTxd1('0');
				f_vd_sendTxd1(':');
				digit = ST_MINUTE.uc_tmpDATEVAL[0]/10;
				f_vd_sendTxd1(0x30+digit);
				value = ST_MINUTE.uc_tmpDATEVAL[0] - (digit*10);
				f_vd_sendTxd1(0x30+value);
				//####################################
			}
			else if (ST_MINUTE.uc_noOfTmpDateVals == 2)
			{
				f_vd_sendTxd1('D');
				f_vd_sendTxd1('F');
				f_vd_sendTxd1('1');
				f_vd_sendTxd1(':');
				digit = ST_MINUTE.ul_tmpDV_timestamp[1]/1000;
				f_vd_sendTxd1(0x30+digit);
				value = ST_MINUTE.ul_tmpDV_timestamp[1] - (digit*1000);
				digit = value/100;
				f_vd_sendTxd1(0x30+digit);
				value = value - (digit*100);
				digit = value/10;
				f_vd_sendTxd1(0x30+digit);
				value = value - (digit*10);
				f_vd_sendTxd1(0x30+value);				
				//####################################
				f_vd_sendTxd1('M');
				f_vd_sendTxd1('N');
				f_vd_sendTxd1('1');
				f_vd_sendTxd1(':');
				digit = ST_MINUTE.uc_tmpDATEVAL[1]/10;
				f_vd_sendTxd1(0x30+digit);
				value = ST_MINUTE.uc_tmpDATEVAL[1] - (digit*10);
				f_vd_sendTxd1(0x30+value);
				//####################################
			}
			else if (ST_MINUTE.uc_noOfTmpDateVals == 3)
			{
				f_vd_sendTxd1('D');
				f_vd_sendTxd1('F');
				f_vd_sendTxd1('2');
				f_vd_sendTxd1(':');
				digit = ST_MINUTE.ul_tmpDV_timestamp[2]/1000;
				f_vd_sendTxd1(0x30+digit);
				value = ST_MINUTE.ul_tmpDV_timestamp[2] - (digit*1000);
				digit = value/100;
				f_vd_sendTxd1(0x30+digit);
				value = value - (digit*100);
				digit = value/10;
				f_vd_sendTxd1(0x30+digit);
				value = value - (digit*10);
				f_vd_sendTxd1(0x30+value);				
				//####################################
				f_vd_sendTxd1('M');
				f_vd_sendTxd1('N');
				f_vd_sendTxd1('2');
				f_vd_sendTxd1(':');
				digit = ST_MINUTE.uc_tmpDATEVAL[2]/10;
				f_vd_sendTxd1(0x30+digit);
				value = ST_MINUTE.uc_tmpDATEVAL[2] - (digit*10);
				f_vd_sendTxd1(0x30+value);
			}
		} //end of if ((ST_MINUTE.uc_noOfTmpDateVals > 0) && !(B_page0_1))
		if ((ST_HOUR.uc_noOfTmpDateVals > 0) && B_page0_1)
		{
			if (ST_HOUR.uc_noOfTmpDateVals == 1)
			{
				f_vd_sendTxd1('H');
				f_vd_sendTxd1('R');
				f_vd_sendTxd1('0');
				f_vd_sendTxd1(':');
				digit = ST_HOUR.uc_tmpDATEVAL[0]/10;
				f_vd_sendTxd1(0x30+digit);
				value = ST_HOUR.uc_tmpDATEVAL[0] - (digit*10);
				f_vd_sendTxd1(0x30+value);
			}
			else if (ST_HOUR.uc_noOfTmpDateVals == 2)
			{
				f_vd_sendTxd1('H');
				f_vd_sendTxd1('R');
				f_vd_sendTxd1('1');
				f_vd_sendTxd1(':');
				digit = ST_HOUR.uc_tmpDATEVAL[1]/10;
				f_vd_sendTxd1(0x30+digit);
				value = ST_HOUR.uc_tmpDATEVAL[1] - (digit*10);
				f_vd_sendTxd1(0x30+value);
			}
			else if (ST_HOUR.uc_noOfTmpDateVals == 3)
			{
				f_vd_sendTxd1('H');
				f_vd_sendTxd1('R');
				f_vd_sendTxd1('2');
				f_vd_sendTxd1(':');
				digit = ST_HOUR.uc_tmpDATEVAL[2]/10;
				f_vd_sendTxd1(0x30+digit);
				value = ST_HOUR.uc_tmpDATEVAL[2] - (digit*10);
				f_vd_sendTxd1(0x30+value);
				//####################################
			}
		} //end of if ((ST_HOUR.uc_noOfTmpDateVals > 0) && B_page0_1)
	} //end of else of if (f_uc_get_dcf77_state() == D_dcf77_state_SYNC_FOUND)	
#endif //FRAME_VALIDITY

f_vd_sendTxd1('e'); /* send end character 'e' */
f_vd_sendTxd1('\r');
f_vd_sendTxd1('\n');
}


#endif

