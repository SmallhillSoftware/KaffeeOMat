/************************************************************************************
*                                                                                   *
*   File Name   : external_interrupts.c                                             *
*   Contents    : functions related with the EXTINT's 0,1 for Doppelkopftimer       *
*   Version     : 1.1                                                               *
*************************************************************************************/ 
#include "globals.h"

/************************************************************************************
Name:        EXTINT0_init
Parameters:  None
Returns:     None
Description: Initial setting of EXTINT0 registers
************************************************************************************/
void f_vd_EXTINTS_init(void)
{
	/* settings for INT0 */
	int0ic              = 0x00; /* interrupt disabled due to prio level 0 */
	EncoderUp_port_dir  = 0;    /* P4.5 = INT0 is input */
	pu11                = 0;    /* P4.5 is not pulled-up */

	/* settings for INT1 */
	int1ic              = 0x00; /* interrupt disabled due to prio level 0 */
	EncoderDwn_port_dir = 0;    /* P1.7 = INT1 is input */
	pu03                = 0;    /* P1.4..P1.7 is not pulled-up */

	/* settings for INT2 */
	int2ic              = 0x00; /* interrupt disabled due to prio level 0 */
	EncoderOk_port_dir  = 0;    /* P3.2 = INT2 is input */
	r1edg         = 1;          /* selects falling edge */
	ir_int2ic     = 0;          /* clear interrupt request flag */
	ilvl0_int2ic  = 1;
	ilvl1_int2ic  = 1;
	ilvl2_int2ic  = 1;          /* interrupt prio level7 */
}

void f_vd_ENCODER_READ(void)
{
  static char c_enc_last = 0x01;
  char c_i = 0;

  if( EncoderDwn_port )
  {
    c_i = 1;
  }

  if( EncoderUp_port )
  {
    c_i ^= 3;				// convert gray to binary
  }

  c_i -= c_enc_last;			// difference new - last

  if( c_i & 1 )                         // bit 0 = value (1)
  {				
    c_enc_last += c_i;			// store new as next last
    C_ENC_DELTA += (c_i & 2) - 1;		// bit 1 = direction (+/-)
  }
}

/************************************************************************************
Name:        interrupt Extint 2
Parameters:  None
Returns:     None
Description: ISR for EXTINT2 (Enter-Key)
************************************************************************************/
#pragma interrupt Extint_2_int
void Extint_2_int(void)
{
unsigned long ul_diff;
	ir_int2ic     = 0;    /* clear interrupt request flag */
	ul_diff = timerDifference (UL_int2_IRQ_timestamp, UL_TIRQ_count1ms);
	if (ul_diff > D_switchDebounceTimeMs) /* switch debouncing */
	{
	   UL_int2_IRQ_timestamp = UL_TIRQ_count1ms; /* update timestamp of sucessfull IRQ */
           B_SWITCH_OK = D_TRUE;
	} //if (diff > cSwitchDebounceTimeMs)
}

/***********************************************************************************/
