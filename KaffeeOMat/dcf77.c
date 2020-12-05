/************************************************************************************
*                                                                                   *
*   File Name   : dcf77.c                                                           *
*   Contents    : Decoder of DCF77-signal for Funkuhr                               *
*   Version     : 1.33                                                              *
*************************************************************************************/ 
#include "globals.h"
#include "dcf77.h"

//global DCF77-variables, changed in ISR
volatile unsigned char UC_DCF77_RECEIVED_FRAME;
volatile unsigned char UC_DCF77_STATE;
volatile unsigned char UC_DCF77_EDGE;
volatile unsigned long UL_DCF77_FALLING_TIME;
volatile unsigned long UL_DCF77_PREVIOUS_FALLING_TIME;
volatile unsigned long UL_DCF77_RISING_TIME;
volatile unsigned long UL_DCF77_BIT_DURATION;
volatile unsigned long UL_DCF77_BIT_DISTANCE;
volatile unsigned char UC_DCF77_FIELD_CNT;
volatile unsigned char UC_DCF77_BIT_FIELD[D_dcf77_MAX_NO_OF_BITS+1];
//global DCF77-variables
volatile struct ST_MINVALS    ST_MINUTE;
volatile struct ST_DATEVALS   ST_HOUR;
unsigned char UC_DATE_MINUTE;
unsigned char UC_DATE_HOUR;
#ifdef DATE_REQUIRED
struct ST_DATEVALS   ST_DAY;
struct ST_DATEVALS   ST_MONTH;
struct ST_DATEVALS   ST_YEAR;
unsigned char UC_DATE_DAY;
unsigned char UC_DATE_MONTH;
unsigned int  UI_DATE_YEAR;
#endif

/************************************************************************************
Name:        f_ui_calc_abs_time_seconds
Parameters:  ui_secs, ui_mins, ui_hours
Returns:     time given by secs, mins and hours in seconds
Description: calculates a time in seconds based on given secs, mins, hours
************************************************************************************/
unsigned int f_ui_calc_abs_time_seconds(unsigned int ui_secs, unsigned int ui_mins, unsigned int ui_hours)
{
unsigned int ui_ret_val;
	ui_ret_val = ui_secs;
	ui_ret_val += 60 * ui_mins;
	ui_ret_val += 3600 * ui_hours;
	return ui_ret_val;
}

/************************************************************************************
Name:        f_ui_calc_mins_from_secs
Parameters:  unsigned long diff_in_secs
Returns:     unsigned int minutes
Description: ???
************************************************************************************/
unsigned int f_ui_calc_mins_from_secs(unsigned long diff_in_secs)
{
#define D_1MIN 60
#define D_2MIN 120
#define D_3MIN 180
#define D_4MIN 240
#define D_5MIN 300
#define D_DELTATOL 10
	if ((diff_in_secs > (D_1MIN-D_DELTATOL)) && (diff_in_secs < (D_1MIN+D_DELTATOL)))
	{
		return 1;
	}
	else if ((diff_in_secs > (D_2MIN-D_DELTATOL)) && (diff_in_secs < (D_2MIN+D_DELTATOL)))
	{
		return 2;
	}
	else if ((diff_in_secs > (D_3MIN-D_DELTATOL)) && (diff_in_secs < (D_3MIN+D_DELTATOL)))
	{
		return 3;
	}
	else if ((diff_in_secs > (D_4MIN-D_DELTATOL)) && (diff_in_secs < (D_4MIN+D_DELTATOL)))
	{
		return 4;
	}
	else if ((diff_in_secs > (D_5MIN-D_DELTATOL)) && (diff_in_secs < (D_5MIN+D_DELTATOL)))
	{
		return 5;
	}
	return 0;
}

/************************************************************************************
Name:        f_vd_DCF77_reset_bit_array
Parameters:  None
Returns:     None
Description: sets all bits in DCF77-bit-field to 0
************************************************************************************/
void f_vd_DCF77_reset_bit_array(void)
{
unsigned char uc_index;
	uc_index = 0;
	while (uc_index < D_dcf77_MAX_NO_OF_BITS)
	{
	      UC_DCF77_BIT_FIELD[uc_index] = 0;
	      uc_index++;
	}
}

/************************************************************************************
Name:        f_uc_get_dcf77_state(void)_
Parameters:  None
Returns:     UC_DCF77_STATE
Description: returns the value of UC_DCF77_STATE
************************************************************************************/
unsigned char f_uc_get_dcf77_state(void)
{
	return UC_DCF77_STATE;
}

/************************************************************************************
Name:        f_vd_DCF77_init
Parameters:  dcf77_tmp_data_reset must be different from zero for reseting old frame data
Returns:     None
Description: Initial setting of DCF77-HW using KEYIN0-IRQ
************************************************************************************/
void f_vd_DCF77_init(_Bool dcf77_tmp_data_reset)
{
	D_dcf77_IRQ_CTRL = 0;                        /* set KeyIn-IRQ-priority to off */
	D_dcf77_IRQ_REQ  = 0;                        /* clear interrupt request flag */
	D_dcf77_IRQ_SRC_ENABLE = 1;                  /* enable the Key-Input-0-IRQ */
	D_dcf77_IRQ_POLARITY = D_dcf77_FALLING_EDGE; /* start acquisition on falling edge on the Key-Input-0-IRQ */
	ki1en = 0;                                   /* disable the Key-Input-1-IRQ */
	ki1pl = 0;                                   /* start acquisition on falling edge on the Key-Input-1-IRQ */
	ki2en = 0;                                   /* disable the Key-Input-2-IRQ */
	ki2pl = 0;                                   /* start acquisition on falling edge on the Key-Input-2-IRQ */
	ki3en = 0;                                   /* disable the Key-Input-3-IRQ */
	ki3pl = 0;                                   /* start acquisition on falling edge on the Key-Input-3-IRQ */    
	pu02  = 0;                                   /* no pull-up of KIx-inputs on P1.0..P1.3 */
	D_dcf77_INPUT_PIN_DIR = 0;                   /* DCF77-pin should be input*/
	UC_DCF77_EDGE = D_dcf77_FALLING_EDGE;
	UC_DCF77_STATE = D_dcf77_state_NO_SIGNAL;
	f_vd_DCF77_reset_bit_array();
	UL_DCF77_FALLING_TIME = 0;
	UL_DCF77_PREVIOUS_FALLING_TIME = 0;
	UL_DCF77_RISING_TIME = 0;
	UL_DCF77_BIT_DURATION = 0;
	UL_DCF77_BIT_DISTANCE = 0;
	UC_DCF77_FIELD_CNT = 0;
	D_dcf77_IRQ_CTRL = 7;                        /* set KeyIn-IRQ-priority to highest priority 7 */
	if (dcf77_tmp_data_reset)
	{
		UC_DCF77_RECEIVED_FRAME = 0;				   /* only to be reset when starting up or fatal errors */
		ST_MINUTE.uc_DATEVAL = 0;
		ST_MINUTE.bt_DATEVAL_VALID = D_FALSE;
		ST_MINUTE.uc_DATEVAL_FRAMENO = 0;
		ST_MINUTE.uc_tmpDATEVAL[0] = 0;
		ST_MINUTE.uc_tmpDATEVAL[1] = 0;
		ST_MINUTE.uc_tmpDATEVAL[2] = 0;
		ST_MINUTE.uc_noOfTmpDateVals = 0;
		ST_MINUTE.uc_DATEVAL_ptyValid = 0;
		ST_MINUTE.ul_tmpDV_timestamp[0] = 0;		
		ST_MINUTE.ul_tmpDV_timestamp[1] = 0;
		ST_MINUTE.ul_tmpDV_timestamp[2] = 0;
		ST_HOUR.uc_DATEVAL = 0;
		ST_HOUR.bt_DATEVAL_VALID = D_FALSE;
		ST_HOUR.uc_DATEVAL_FRAMENO = 0;
		ST_HOUR.uc_tmpDATEVAL[0] = 0;
		ST_HOUR.uc_tmpDATEVAL[1] = 0;
		ST_HOUR.uc_tmpDATEVAL[2] = 0;
		ST_HOUR.uc_noOfTmpDateVals = 0;
		ST_HOUR.uc_DATEVAL_ptyValid = 0;
		#ifdef DATE_REQUIRED
			ST_DAY.uc_DATEVAL = 0;
			ST_DAY.bt_DATEVAL_VALID = D_FALSE;
			ST_DAY.uc_DATEVAL_FRAMENO = 0;
			ST_MONTH.uc_DATEVAL = 0;
			ST_MONTH.bt_DATEVAL_VALID = D_FALSE;
			ST_MONTH.uc_DATEVAL_FRAMENO = 0;
			ST_YEAR.uc_DATEVAL = 0;
			ST_YEAR.bt_DATEVAL_VALID = D_FALSE;
			ST_YEAR.uc_DATEVAL_FRAMENO = 0;
			ST_DAY.uc_DATEVAL = 0;
			ST_DAY.bt_DATEVAL_VALID = D_FALSE;
			ST_DAY.uc_DATEVAL_FRAMENO = 0;
			ST_DAY.uc_tmpDATEVAL[0] = 0;
			ST_DAY.uc_tmpDATEVAL[1] = 0;
			ST_DAY.uc_tmpDATEVAL[2] = 0;
			ST_DAY.uc_noOfTmpDateVals = 0;
			ST_DAY.uc_DATEVAL_ptyValid = 0;
			ST_MONTH.uc_DATEVAL = 0;
			ST_MONTH.bt_DATEVAL_VALID = D_FALSE;
			ST_MONTH.uc_DATEVAL_FRAMENO = 0;
			ST_MONTH.uc_tmpDATEVAL[0] = 0;
			ST_MONTH.uc_tmpDATEVAL[1] = 0;
			ST_MONTH.uc_tmpDATEVAL[2] = 0;
			ST_MONTH.uc_noOfTmpDateVals = 0;
			ST_MONTH.uc_DATEVAL_ptyValid = 0;
			ST_YEAR.uc_DATEVAL = 0;
			ST_YEAR.bt_DATEVAL_VALID = D_FALSE;
			ST_YEAR.uc_DATEVAL_FRAMENO = 0;
			ST_YEAR.uc_tmpDATEVAL[0] = 0;
			ST_YEAR.uc_tmpDATEVAL[1] = 0;
			ST_YEAR.uc_tmpDATEVAL[2] = 0;
			ST_YEAR.uc_noOfTmpDateVals = 0;
			ST_YEAR.uc_DATEVAL_ptyValid = 0;
		#endif //DATE_REQUIRED
	} //end of if (dcf77_tmp_data_reset)
}
/************************************************************************************
Name:        f_vd_DCF77_disableIRQ
Parameters:  None
Returns:     None
Description: Disable the DCF77-IRQ
************************************************************************************/
void f_vd_DCF77_disableIRQ(void)
{
    D_dcf77_IRQ_CTRL = 0;                        /* set KeyIn-IRQ-priority to off */
    D_dcf77_IRQ_REQ  = 0;                        /* clear interrupt request flag */
}

/************************************************************************************
Name:        f_b_check_dcf77_sync_pulse
Parameters:  UL_DCF77_BIT_DISTANCE
Returns:     FALSE or TRUE dependent if the distance indicates a sync-pulse 
Description: Checks if the bit distance indicates a DCF77-Sync-Pulse with 2000ms
************************************************************************************/
_Bool f_b_check_dcf77_sync_pulse(unsigned long ul_dcf77_bit_distance_time)
{
_Bool b_ret_val;
      if ( (ul_dcf77_bit_distance_time < (unsigned long)((unsigned long)D_dcf77_SYNC_PULSE_LENGTH + (unsigned long)D_dcf77_SYNC_PULSE_TOLERANCE)) &&
           (ul_dcf77_bit_distance_time > (unsigned long)((unsigned long)D_dcf77_SYNC_PULSE_LENGTH - (unsigned long)D_dcf77_SYNC_PULSE_TOLERANCE))
         )
      {
         b_ret_val = D_TRUE;
      }
      else
      {
         b_ret_val = D_FALSE;
      }
      return b_ret_val;	 
}

/************************************************************************************
Name:        f_uc_dcf77_bit_decoder
Parameters:  UL_DCF77_BIT_DURATION
Returns:     0 or 1 if the duration indicates a 0- or 1-bit, 3 if the distance
             to the previous pulse is too short
Description: Checks if the bit duration indicates a 0-pulse = 100ms
                                               or a 1-pulse = 200ms     
************************************************************************************/
unsigned char f_uc_dcf77_bit_decoder(unsigned long ul_dcf77_bit_duration_time, unsigned long ul_dcf77_bit_distance_time)
{
unsigned char uc_ret_val;
		if (ul_dcf77_bit_distance_time < ((unsigned long)D_dcf77_SHORTEST_BIT_DISTANCE))
		{
		   uc_ret_val = 3;
		}
		else
		{   
		   if ( (ul_dcf77_bit_duration_time < ((unsigned long)D_dcf77_LOW_PULSE_HI_LIMIT))  &&
                        (ul_dcf77_bit_duration_time > ((unsigned long)D_dcf77_LOW_PULSE_LO_LIMIT))
                      )
                   {
	              uc_ret_val = 0;
                   }
		   else
		   {
		      uc_ret_val = 1;
		   }
		} //else   
		return uc_ret_val;
}

/************************************************************************************
Name:        f_b_check_dcf77_even_parity
Parameters:  uc_start_bit, uc_end_bit, uc_parity_to_check
Returns:     FALSE or TRUE dependent if the bits have even parity
Description: calculates an even-parity from the start to the end bit and compares
             to the received parity bit
************************************************************************************/
_Bool f_b_check_dcf77_even_parity(unsigned char uc_start_bit, unsigned char uc_end_bit, unsigned char uc_parity_to_check)
{
unsigned char uc_index;
unsigned char uc_bit_count;
unsigned char uc_calculated_parity;
	uc_bit_count = 0;
	uc_index = uc_start_bit;
	while (uc_index <= uc_end_bit)
	{
	      uc_bit_count += UC_DCF77_BIT_FIELD[uc_index];
	      uc_index++;
	} //end of while(uc_index <= uc_end_bit)
	if ((uc_bit_count % 2) == 0) {uc_calculated_parity = 0;}
	else {uc_calculated_parity = 1;}
	if (uc_calculated_parity == uc_parity_to_check) { return D_TRUE; }
	else { return D_FALSE; }
}

/************************************************************************************
Name:        f_uc_dcf77_get_date_minute
Parameters:  none
Returns:     0 to 59 belonging to the received minute value
Description: returns the minute from the internal variable
************************************************************************************/
unsigned char f_uc_dcf77_get_date_minute(void)
{
	return UC_DATE_MINUTE;
}

/************************************************************************************
Name:        f_uc_dcf77_get_date_hour
Parameters:  none
Returns:     0 to 23 belonging to the received hour value
Description: returns the hour from the internal variable
************************************************************************************/
unsigned char f_uc_dcf77_get_date_hour(void)
{
	return UC_DATE_HOUR;
}

#ifdef DATE_REQUIRED
/************************************************************************************
Name:        f_uc_dcf77_get_date_day
Parameters:  none
Returns:     1 to 31 belonging to the received day value
Description: returns the day from the internal variable
************************************************************************************/
unsigned char f_uc_dcf77_get_date_day(void)
{
	return UC_DATE_DAY;
}

/************************************************************************************
Name:        f_uc_dcf77_get_date_month
Parameters:  none
Returns:     1 to 12 belonging to the received month value
Description: returns the month from the internal variable
************************************************************************************/
unsigned char f_uc_dcf77_get_date_month(void)
{
	return UC_DATE_MONTH;
}

/************************************************************************************
Name:        f_ui_dcf77_get_date_year
Parameters:  none
Returns:     2000 to 2099 belonging to the received year value
Description: returns the year from the internal variable
************************************************************************************/
unsigned int f_ui_dcf77_get_date_year(void)
{
	return UI_DATE_YEAR;
}

#endif

/************************************************************************************
Name:        f_uc_dcf77_get_minute
Parameters:  none
Returns:     0 to 59 belonging to the received minute value
Description: calculates and returns the minute from the received dcf77-bit-field
************************************************************************************/
unsigned char f_uc_dcf77_get_minute(void)
{
unsigned char uc_ret_val;
	uc_ret_val  =  UC_DCF77_BIT_FIELD[21];
	uc_ret_val += (UC_DCF77_BIT_FIELD[22] *  2);
	uc_ret_val += (UC_DCF77_BIT_FIELD[23] *  4);
	uc_ret_val += (UC_DCF77_BIT_FIELD[24] *  8);
	uc_ret_val += (UC_DCF77_BIT_FIELD[25] * 10);
	uc_ret_val += (UC_DCF77_BIT_FIELD[26] * 20);
	uc_ret_val += (UC_DCF77_BIT_FIELD[27] * 40);
	return uc_ret_val;
}

/************************************************************************************
Name:        f_uc_dcf77_get_hour
Parameters:  none
Returns:     0 to 23 belonging to the received hour value
Description: calculates and returns the hour from the received dcf77-bit-field
************************************************************************************/
unsigned char f_uc_dcf77_get_hour(void)
{
unsigned char uc_ret_val;
	uc_ret_val  =  UC_DCF77_BIT_FIELD[29];
	uc_ret_val += (UC_DCF77_BIT_FIELD[30] *  2);
	uc_ret_val += (UC_DCF77_BIT_FIELD[31] *  4);
	uc_ret_val += (UC_DCF77_BIT_FIELD[32] *  8);
	uc_ret_val += (UC_DCF77_BIT_FIELD[33] * 10);
	uc_ret_val += (UC_DCF77_BIT_FIELD[34] * 20);
	/*
	//time is sent in MEZ or MESZ meaning local German time
	if ((UC_DCF77_BIT_FIELD[17] == 0) && (UC_DCF77_BIT_FIELD[18] == 1)) //MEZ
	{
	   uc_ret_val += 1; //hour in bits is UTC, German time in Winter is +1h
	}
	else
	{
	   uc_ret_val += 2; //hour in bits is UTC, German time in Summer is +2h	
	}
	*/
	return uc_ret_val;
}

#ifdef DATE_REQUIRED
/************************************************************************************
Name:        f_uc_dcf77_get_day
Parameters:  none
Returns:     1 to 31 belonging to the received day value
Description: calculates and returns the minute from the received dcf77-bit-field
************************************************************************************/
unsigned char f_uc_dcf77_get_day(void)
{
unsigned char uc_ret_val;
	uc_ret_val  =  UC_DCF77_BIT_FIELD[36];
	uc_ret_val += (UC_DCF77_BIT_FIELD[37] *  2);
	uc_ret_val += (UC_DCF77_BIT_FIELD[38] *  4);
	uc_ret_val += (UC_DCF77_BIT_FIELD[39] *  8);
	uc_ret_val += (UC_DCF77_BIT_FIELD[40] * 10);
	uc_ret_val += (UC_DCF77_BIT_FIELD[41] * 20);
	return uc_ret_val;
}

/************************************************************************************
Name:        f_uc_dcf77_get_month
Parameters:  none
Returns:     1 to 12 belonging to the received month value
Description: calculates and returns the minute from the received dcf77-bit-field
************************************************************************************/
unsigned char f_uc_dcf77_get_month(void)
{
unsigned char uc_ret_val;
	uc_ret_val  =  UC_DCF77_BIT_FIELD[45];
	uc_ret_val += (UC_DCF77_BIT_FIELD[46] *  2);
	uc_ret_val += (UC_DCF77_BIT_FIELD[47] *  4);
	uc_ret_val += (UC_DCF77_BIT_FIELD[48] *  8);
	uc_ret_val += (UC_DCF77_BIT_FIELD[49] * 10);
	return uc_ret_val;
}

/************************************************************************************
Name:        f_uc_dcf77_get_year
Parameters:  none
Returns:     0 to 99 belonging to the received year value
Description: calculates and returns the year from the received dcf77-bit-field
************************************************************************************/
unsigned int f_uc_dcf77_get_year(void)
{
unsigned char uc_ret_val;
	uc_ret_val  =  UC_DCF77_BIT_FIELD[50];
	uc_ret_val += (UC_DCF77_BIT_FIELD[51] *  2);
	uc_ret_val += (UC_DCF77_BIT_FIELD[52] *  4);
	uc_ret_val += (UC_DCF77_BIT_FIELD[53] *  8);
	uc_ret_val += (UC_DCF77_BIT_FIELD[54] * 10);
	uc_ret_val += (UC_DCF77_BIT_FIELD[55] * 20);
	uc_ret_val += (UC_DCF77_BIT_FIELD[56] * 40);
	uc_ret_val += (UC_DCF77_BIT_FIELD[57] * 80);
	return uc_ret_val;
}
#endif

/************************************************************************************
Name:        f_uc_check_dcf77_frame_validity
Parameters:  unsigned int ui_secs, unsigned int ui_mins, unsigned int ui_hours
Returns:     check frame for validity
Description: checks the fixed bits in the bit field [bit0 = 0, bit20 = 1],
             the different bits 17,18 for MEZ, MESZ, the correct bit distance and
             the parity of minute and hour and sets the state either to frame valid
	     		 or no signal to repeat the receiving of another frame
************************************************************************************/
unsigned char f_uc_check_dcf77_frame_validity(unsigned int ui_secs, unsigned int ui_mins, unsigned int ui_hours)
{
unsigned char uc_index;
unsigned char tmp_ret_val;
unsigned char tmp_rcv_val;
_Bool         tmp_par_val;
	if (UC_DCF77_STATE == D_dcf77_state_FRAME_RECEIVED)
	{
		uc_index = 0;
		tmp_ret_val = 0;
		tmp_rcv_val = 0;
		if	(UC_DCF77_RECEIVED_FRAME == 25)
		{
			f_vd_DCF77_init(D_TRUE);
			return D_dcf77_25_wrong_frames;
		} //end of if	(UC_DCF77_RECEIVED_FRAME == 25)
		while (uc_index < D_dcf77_MAX_NO_OF_USED_BITS)
		{
			if (UC_DCF77_BIT_FIELD[uc_index] == 3)
	      {
	         f_vd_DCF77_init(D_FALSE);
	         return D_dcf77_invalid_bit_distance;
	      } //end of if (UC_DCF77_BIT_FIELD[uc_index] == 3)
	      uc_index++;
		} //end of while (uc_index < D_dcf77_MAX_NO_OF_USED_BITS)

		if (UC_DCF77_BIT_FIELD[0]  != 0)
		{
	   	f_vd_DCF77_init(D_FALSE);
	   	return D_dcf77_invalid_frame_start;
		} //end of if (UC_DCF77_BIT_FIELD[0]  != 0)

		if (UC_DCF77_BIT_FIELD[17] == UC_DCF77_BIT_FIELD[18])
		{
	   	f_vd_DCF77_init(D_FALSE);
	   	return D_dcf77_invalid_mez_mesz;
		} //end of if (UC_DCF77_BIT_FIELD[17] == UC_DCF77_BIT_FIELD[18])
	
		if (UC_DCF77_BIT_FIELD[20] != 1)
		{
	   	f_vd_DCF77_init(D_FALSE);
	   	return D_dcf77_invalid_time_start;
		} //end of if (UC_DCF77_BIT_FIELD[20] != 1)		

		/* all date values will be evaluated and stored from here */		
		/* start of minute processing */
		if (ST_MINUTE.bt_DATEVAL_VALID == D_FALSE)
		{
			tmp_rcv_val = f_uc_dcf77_get_minute();
			tmp_par_val = f_b_check_dcf77_even_parity(21, 27, UC_DCF77_BIT_FIELD[28]);		
			if (tmp_rcv_val > 59)
			{
				tmp_ret_val += D_dcf77_invalid_minute_val;
			} //end of if (tmp_rcv_val > 59)
			else
			{
				if (ST_MINUTE.uc_noOfTmpDateVals < 3)	   	
				{
					ST_MINUTE.uc_tmpDATEVAL[ST_MINUTE.uc_noOfTmpDateVals] = tmp_rcv_val;
					ST_MINUTE.ul_tmpDV_timestamp[ST_MINUTE.uc_noOfTmpDateVals] = f_ui_calc_abs_time_seconds(ui_secs, ui_mins, ui_hours);
					ST_MINUTE.uc_DATEVAL_ptyValid += tmp_par_val;
					ST_MINUTE.uc_noOfTmpDateVals++;
					if (ST_MINUTE.uc_noOfTmpDateVals == 3)
					{
						if ( ((ST_MINUTE.uc_tmpDATEVAL[0]+f_ui_calc_mins_from_secs(ST_MINUTE.ul_tmpDV_timestamp[1]-ST_MINUTE.ul_tmpDV_timestamp[0])) == ST_MINUTE.uc_tmpDATEVAL[1]) &&
					  	  	  ((ST_MINUTE.uc_tmpDATEVAL[1]+f_ui_calc_mins_from_secs(ST_MINUTE.ul_tmpDV_timestamp[2]-ST_MINUTE.ul_tmpDV_timestamp[1])) == ST_MINUTE.uc_tmpDATEVAL[2]) &&
					   	  (ST_MINUTE.uc_DATEVAL_ptyValid >= 3)
							)
						{
							/* all parity bits must be correct and the minute values must have the correct minute distance */					
							ST_MINUTE.uc_DATEVAL = ST_MINUTE.uc_tmpDATEVAL[2];
							ST_MINUTE.bt_DATEVAL_VALID = D_TRUE;
							ST_MINUTE.uc_DATEVAL_FRAMENO = UC_DCF77_RECEIVED_FRAME;
						} //end of if ( ((ST_MINUTE.uc_tmpDATEVAL[0....
						else
						{
							ST_MINUTE.uc_DATEVAL_ptyValid = 0;					
							ST_MINUTE.uc_noOfTmpDateVals = 0;
							tmp_ret_val += D_dcf77_3_min_vals_not_match;
						} //end of else of if ( ((ST_MINUTE.uc_tmpDATEVAL[0
					} //end of if (ST_MINUTE.uc_noOfTmpDateVals == 3)
					else
					{
						tmp_ret_val += D_dcf77_less_than_3_min_vals;
					} //end of else of if (ST_MINUTE.uc_noOfTmpDateVals == 3)
				}
			} //end of else of if (tmp_rcv_val > 59)
			if (!tmp_par_val)
			{
				tmp_ret_val += D_dcf77_invalid_minute_parity;
			}
		} //end of if (ST_MINUTE.bt_DATEVAL_VALID = D_FALSE)
		/* end of minute processing */
		
		/* start of hour processing */
		if (ST_HOUR.bt_DATEVAL_VALID == D_FALSE)
		{
			tmp_rcv_val = f_uc_dcf77_get_hour();
			tmp_par_val = f_b_check_dcf77_even_parity(29, 34, UC_DCF77_BIT_FIELD[35]);		
			if (tmp_rcv_val > 23)
			{
				tmp_ret_val += D_dcf77_invalid_hour_val;
			} //end of if (tmp_rcv_val > 23)
			else
			{
				if (ST_HOUR.uc_noOfTmpDateVals < 3)	   	
				{
					ST_HOUR.uc_tmpDATEVAL[ST_HOUR.uc_noOfTmpDateVals] = tmp_rcv_val;
					ST_HOUR.uc_DATEVAL_ptyValid += tmp_par_val;
					ST_HOUR.uc_noOfTmpDateVals++;
					if (ST_HOUR.uc_noOfTmpDateVals == 3)
					{
						if ( (ST_HOUR.uc_tmpDATEVAL[0] == ST_HOUR.uc_tmpDATEVAL[1]) &&
						  	  (ST_HOUR.uc_tmpDATEVAL[1] == ST_HOUR.uc_tmpDATEVAL[2]) &&
						  	  (ST_HOUR.uc_DATEVAL_ptyValid == 3)
							)
						{
							/* all parity bits must be correct and the 3 stored hour values must be the same value */					
							ST_HOUR.uc_DATEVAL = ST_HOUR.uc_tmpDATEVAL[2];
							ST_HOUR.bt_DATEVAL_VALID = D_TRUE;
							ST_HOUR.uc_DATEVAL_FRAMENO = UC_DCF77_RECEIVED_FRAME;
						} //end of if ( (ST_HOUR.uc_tmpDATEVAL[0...
						else
						{
							ST_HOUR.uc_DATEVAL_ptyValid = 0;					
							ST_HOUR.uc_noOfTmpDateVals = 0;
							tmp_ret_val += D_dcf77_3_hr_vals_not_match;
						} //end of else of if ( (ST_HOUR.uc_tmpDATEVAL[0...
					} //end of if (ST_HOUR.uc_noOfTmpDateVals == 3)
					else
					{
						tmp_ret_val += D_dcf77_less_than_3_hr_vals;
					} //end of else of if (ST_HOUR.uc_noOfTmpDateVals == 3)
				} //end of if (ST_HOUR.uc_noOfTmpDateVals < 3)
			} //end of else of if (tmp_rcv_val > 23)
			if (!tmp_par_val)
			{
				tmp_ret_val += D_dcf77_invalid_hour_parity;
			}
		} //end of if (ST_HOUR.bt_DATEVAL_VALID = D_FALSE)
		/* end of hour processing */
		
		#ifdef DATE_REQUIRED
		tmp_par_val = f_b_check_dcf77_even_parity(36, 57, UC_DCF77_BIT_FIELD[58]);
		/* start of day processing */
		if (ST_DAY.bt_DATEVAL_VALID == D_FALSE)
		{
			tmp_rcv_val = f_uc_dcf77_get_day();
			if ( (tmp_rcv_val > 31) || (tmp_rcv_val < 1) )
			{
				tmp_ret_val += D_dcf77_invalid_day_val;
			} //end of if ( (tmp_rcv_val > 31) || (tmp_rcv_val < 1) )
			else
			{
				if (ST_DAY.uc_noOfTmpDateVals < 3)
				{
					ST_DAY.uc_tmpDATEVAL[ST_DAY.uc_noOfTmpDateVals] = tmp_rcv_val;
					ST_DAY.uc_DATEVAL_ptyValid += tmp_par_val;
					ST_DAY.uc_noOfTmpDateVals++;
					if (ST_DAY.uc_noOfTmpDateVals == 3)
					{
						if ( (ST_DAY.uc_tmpDATEVAL[0] == ST_DAY.uc_tmpDATEVAL[1]) &&
						  	  (ST_DAY.uc_tmpDATEVAL[1] == ST_DAY.uc_tmpDATEVAL[2]) &&
						  	  (ST_DAY.uc_DATEVAL_ptyValid == 3)
							)
						{
							/* all parity bits must be correct and the 3 stored hour values must be the same value */					
							ST_DAY.uc_DATEVAL = ST_DAY.uc_tmpDATEVAL[2];
							ST_DAY.bt_DATEVAL_VALID = D_TRUE;
							ST_DAY.uc_DATEVAL_FRAMENO = UC_DCF77_RECEIVED_FRAME;
						} //end of if ( (ST_DAY.uc_tmpDATEVAL[0...
						else
						{
							ST_DAY.uc_DATEVAL_ptyValid = 0;					
							ST_DAY.uc_noOfTmpDateVals = 0;
							tmp_ret_val += D_dcf77_3_dy_vals_not_match;
						} //end of else of if ( (ST_DAY.uc_tmpDATEVAL[0...
					} //end of if (ST_DAY.uc_noOfTmpDateVals == 3)
					else
					{
						tmp_ret_val += D_dcf77_less_than_3_dy_vals;
					} //end of else of if (ST_DAY.uc_noOfTmpDateVals == 3)
				} //end of if (ST_DAY.uc_noOfTmpDateVals < 3)
			} //end of else of if ( (tmp_rcv_val > 31) || (tmp_rcv_val < 1) )
		} //end of if (ST_DAY.bt_DATEVAL_VALID == D_FALSE)
		/* end of day processing */
		
		/* start of month processing */
		if (ST_MONTH.bt_DATEVAL_VALID == D_FALSE)
		{
			tmp_rcv_val = f_uc_dcf77_get_month();
			if ( (tmp_rcv_val > 12) || (tmp_rcv_val < 1) )
			{
				tmp_ret_val += D_dcf77_invalid_month_val;
			} //end of if ( (tmp_rcv_val > 12) || (tmp_rcv_val < 1) )
			else
			{
				if (ST_MONTH.uc_noOfTmpDateVals < 3)
				{
					ST_MONTH.uc_tmpDATEVAL[ST_MONTH.uc_noOfTmpDateVals] = tmp_rcv_val;
					ST_MONTH.uc_DATEVAL_ptyValid += tmp_par_val;
					ST_MONTH.uc_noOfTmpDateVals++;
					if (ST_MONTH.uc_noOfTmpDateVals == 3)
					{
						if ( (ST_MONTH.uc_tmpDATEVAL[0] == ST_MONTH.uc_tmpDATEVAL[1]) &&
						  	  (ST_MONTH.uc_tmpDATEVAL[1] == ST_MONTH.uc_tmpDATEVAL[2]) &&
						  	  (ST_MONTH.uc_DATEVAL_ptyValid == 3)
							)
						{
							/* all parity bits must be correct and the 3 stored hour values must be the same value */					
							ST_MONTH.uc_DATEVAL = ST_MONTH.uc_tmpDATEVAL[2];
							ST_MONTH.bt_DATEVAL_VALID = D_TRUE;
							ST_MONTH.uc_DATEVAL_FRAMENO = UC_DCF77_RECEIVED_FRAME;
						} //end of if ( (ST_MONTH.uc_tmpDATEVAL[0...
						else
						{
							ST_MONTH.uc_DATEVAL_ptyValid = 0;					
							ST_MONTH.uc_noOfTmpDateVals = 0;
							tmp_ret_val += D_dcf77_3_mn_vals_not_match;
						} //end of else of if ( (ST_MONTH.uc_tmpDATEVAL[0...
					} //end of if (ST_MONTH.uc_noOfTmpDateVals == 3)
					else
					{
						tmp_ret_val += D_dcf77_less_than_3_mn_vals;
					} //end of else of if (ST_MONTH.uc_noOfTmpDateVals == 3)
				} //end of if (ST_MONTH.uc_noOfTmpDateVals < 3)
			} //end of else of if ( (tmp_rcv_val > 12) || (tmp_rcv_val < 1) )
		} //end of if (ST_MONTH.bt_DATEVAL_VALID == D_FALSE)
		/* end of month processing */
		
		/* start of year processing */
		if (ST_YEAR.bt_DATEVAL_VALID == D_FALSE)
		{
			tmp_rcv_val = f_uc_dcf77_get_year();	
			if (tmp_rcv_val > 99)
			{
				tmp_ret_val += D_dcf77_invalid_year_val;
			} //end of if (tmp_rcv_val > 99)
			else
			{
				if (ST_YEAR.uc_noOfTmpDateVals < 3)
				{
					ST_YEAR.uc_tmpDATEVAL[ST_YEAR.uc_noOfTmpDateVals] = tmp_rcv_val;
					ST_YEAR.uc_DATEVAL_ptyValid += tmp_par_val;
					ST_YEAR.uc_noOfTmpDateVals++;
					if (ST_YEAR.uc_noOfTmpDateVals == 3)
					{
						if ( (ST_YEAR.uc_tmpDATEVAL[0] == ST_YEAR.uc_tmpDATEVAL[1]) &&
						  	  (ST_YEAR.uc_tmpDATEVAL[1] == ST_YEAR.uc_tmpDATEVAL[2]) &&
						  	  (ST_YEAR.uc_DATEVAL_ptyValid == 3)
							)
						{
							/* all parity bits must be correct and the 3 stored hour values must be the same value */					
							ST_YEAR.uc_DATEVAL = ST_YEAR.uc_tmpDATEVAL[2];
							ST_YEAR.bt_DATEVAL_VALID = D_TRUE;
							ST_YEAR.uc_DATEVAL_FRAMENO = UC_DCF77_RECEIVED_FRAME;
						} //end of if ( (ST_YEAR.uc_tmpDATEVAL[0...
						else
						{
							ST_YEAR.uc_DATEVAL_ptyValid = 0;					
							ST_YEAR.uc_noOfTmpDateVals = 0;
							tmp_ret_val += D_dcf77_3_yr_vals_not_match;
						} //end of else of if ( (ST_YEAR.uc_tmpDATEVAL[0...
					} //end of if (ST_YEAR.uc_noOfTmpDateVals == 3)
					else
					{
						tmp_ret_val += D_dcf77_less_than_3_yr_vals;
					} //end of else of if (ST_MONTH.uc_noOfTmpDateVals == 3)
				} //end of if (ST_YEAR.uc_noOfTmpDateVals < 3)
			} //end of else of if (tmp_rcv_val > 99)
		} //end of if (ST_YEAR.bt_DATEVAL_VALID == D_FALSE)
		/* end of year processing */
		#endif //DATE_REQUIRED
		if (tmp_ret_val != 0)
		{
			f_vd_DCF77_init(D_FALSE);		
			return tmp_ret_val;
		} //end of if (tmp_ret_val != 0)
		UC_DCF77_STATE = D_dcf77_state_FRAME_CHECKED;
		return D_dcf77_valid_reception;
	} //end of if (UC_DCF77_STATE == D_dcf77_state_FRAME_RECEIVED)
	return D_dcf77_frame_eval_too_early;
}

/************************************************************************************
Name:        f_uc_check_dcf77_date_validity
Parameters:  none
Returns:     check date for validity
Description: checks the validity of minutes, hours, day, month
************************************************************************************/
unsigned char f_uc_check_dcf77_date_validity(void)
{
	if (UC_DCF77_STATE == D_dcf77_state_FRAME_CHECKED)	
	{
		if ( ST_MINUTE.bt_DATEVAL_VALID &&
			  ST_HOUR.bt_DATEVAL_VALID
		#ifdef DATE_REQUIRED
			  && ST_DAY.bt_DATEVAL_VALID &&
			  ST_MONTH.bt_DATEVAL_VALID &&
			  ST_YEAR.bt_DATEVAL_VALID
		#endif		  
			)
		{
			/* There is a valid minute, hour(, day, month, year) received */		
			if (ST_MINUTE.uc_DATEVAL_FRAMENO == ST_HOUR.uc_DATEVAL_FRAMENO)
			{
				/* valid minute and hour are received in the same dcf77-frame */
				UC_DATE_MINUTE  = ST_MINUTE.uc_DATEVAL;
				UC_DATE_HOUR	 = ST_HOUR.uc_DATEVAL;
				#ifdef DATE_REQUIRED
					UC_DATE_DAY     = ST_DAY.uc_DATEVAL;
					UC_DATE_MONTH   = ST_MONTH.uc_DATEVAL;
					UI_DATE_YEAR    = ST_YEAR.uc_DATEVAL + D_dcf77_YEAR_OFFSET;
				#endif
				UC_DCF77_STATE = D_dcf77_state_DATE_VALID;
				return D_date_valid;
			} //end of if (ST_MINUTE.uc_DATEVAL_FRAMENO == ST_HOUR.uc_DATEVAL_FRAMENO)
			else if (ST_MINUTE.uc_DATEVAL_FRAMENO < ST_HOUR.uc_DATEVAL_FRAMENO)
			{
				/* a valid minute was received in an earlier dcf77-frame than the valid hour */
				UC_DATE_MINUTE = ST_MINUTE.uc_DATEVAL + (2*(ST_HOUR.uc_DATEVAL_FRAMENO - ST_MINUTE.uc_DATEVAL_FRAMENO));
				UC_DATE_HOUR = ST_HOUR.uc_DATEVAL;
				while (UC_DATE_MINUTE > 59)
				{
					UC_DATE_MINUTE -= 60;
					UC_DATE_HOUR++;
				}
				/* Hier noch abfangen, dass Stunde groesser als 23 wird */
				#ifdef DATE_REQUIRED
					UC_DATE_DAY     = ST_DAY.uc_DATEVAL;
					UC_DATE_MONTH   = ST_MONTH.uc_DATEVAL;
					UI_DATE_YEAR    = ST_YEAR.uc_DATEVAL + D_dcf77_YEAR_OFFSET;
				#endif
				UC_DCF77_STATE = D_dcf77_state_DATE_VALID;
				return D_date_valid;
			} //end of else if (ST_MINUTE.uc_DATEVAL_FRAMENO < ST_HOUR.uc_DATEVAL_FRAMENO)
			else
			{
				/* a valid minute was received in an later dcf77-frame than the valid hour */
				if ((ST_MINUTE.uc_DATEVAL_FRAMENO - ST_HOUR.uc_DATEVAL_FRAMENO) > ST_MINUTE.uc_DATEVAL)
				{
					UC_DATE_HOUR = ST_HOUR.uc_DATEVAL + 1;	
				}
				/* Hier noch abfangen, dass Stunde groesser als 23 wird */
				UC_DATE_MINUTE = ST_MINUTE.uc_DATEVAL;
				#ifdef DATE_REQUIRED
					UC_DATE_DAY     = ST_DAY.uc_DATEVAL;
					UC_DATE_MONTH   = ST_MONTH.uc_DATEVAL;
					UI_DATE_YEAR    = ST_YEAR.uc_DATEVAL + D_dcf77_YEAR_OFFSET;
				#endif
				UC_DCF77_STATE = D_dcf77_state_DATE_VALID;
				return D_date_valid;
			} //end of else of if (ST_MINUTE.uc_DATEVAL_FRAMENO < ST_HOUR.uc_DATEVAL_FRAMENO)
		} //end of if ( ST_MINUTE.bt_DATEVAL_VALID &&....
		else
		{
			f_vd_DCF77_init(D_FALSE);
			return D_date_invalid;
		} //end of else of if ( ST_MINUTE.bt_DATEVAL_VALID &&
	} //end of if (UC_DCF77_STATE == D_dcf77_state_FRAME_CHECKED)
	return D_date_eval_too_early;
}

/************************************************************************************
Name:        interrupt DCF77 using KEYIN0-IRQ
Parameters:  None
Returns:     None
Description: an external HW-IRQ is needed, which triggers on rising and falling 
             edge, configurable but only one at a time
************************************************************************************/
#pragma interrupt dcf77_keyin0_irq
void dcf77_keyin0_irq(void)
{
	asm("FCLR I"); //disable all IRQ's
	D_dcf77_IRQ_REQ = 0; //clear interrupt request flag
	if (UC_DCF77_STATE < D_dcf77_state_FRAME_RECEIVED)
	{
	   if (UC_DCF77_EDGE == D_dcf77_FALLING_EDGE)
	   {
	      UL_DCF77_FALLING_TIME = UL_TIRQ_count1ms; //store timer value of current falling edge
	      UC_DCF77_EDGE = D_dcf77_RISING_EDGE;
	      D_dcf77_IRQ_REQ = 1;
	      D_dcf77_IRQ_POLARITY = D_dcf77_RISING_EDGE; //change IRQ-trigger-polarity
	      D_dcf77_IRQ_REQ = 0;
	   } //end of if (UC_DCF77_EDGE == D_dcf77_FALLING_EDGE)
	   else //rising edge
	   {
	      UL_DCF77_RISING_TIME = UL_TIRQ_count1ms; //store timer value of current rising edge
	      UC_DCF77_EDGE = D_dcf77_FALLING_EDGE;
	      D_dcf77_IRQ_REQ = 1;
	      D_dcf77_IRQ_POLARITY = D_dcf77_FALLING_EDGE; //change IRQ-trigger-polarity
	      D_dcf77_IRQ_REQ = 0;
	      if (UC_DCF77_STATE == D_dcf77_state_NO_SIGNAL) //no previous complete bit received
	      {
				UC_DCF77_STATE = D_dcf77_state_BIT_RECEIVED; //now at least 1 complete bit was received
		      UL_DCF77_PREVIOUS_FALLING_TIME = UL_DCF77_FALLING_TIME;
	      } //end of if (UC_DCF77_STATE == D_dcf77_state_NO_SIGNAL)
	      else
	      {
				UL_DCF77_BIT_DURATION = timerDifference(UL_DCF77_FALLING_TIME, UL_DCF77_RISING_TIME);
				UL_DCF77_BIT_DISTANCE = timerDifference(UL_DCF77_PREVIOUS_FALLING_TIME, UL_DCF77_FALLING_TIME);
				UL_DCF77_PREVIOUS_FALLING_TIME = UL_DCF77_FALLING_TIME;
				if (f_b_check_dcf77_sync_pulse(UL_DCF77_BIT_DISTANCE))
				{
					UC_DCF77_STATE = D_dcf77_state_SYNC_FOUND;
					UC_DCF77_FIELD_CNT = 0;
				} //end of if (f_b_check_dcf77_sync_pulse(UL_DCF77_BIT_DISTANCE))
				if ( (UC_DCF77_STATE == D_dcf77_state_SYNC_FOUND) &&
					 (UC_DCF77_FIELD_CNT < (D_dcf77_MAX_NO_OF_BITS+1))
					)
				{
					UC_DCF77_BIT_FIELD[UC_DCF77_FIELD_CNT] = f_uc_dcf77_bit_decoder(UL_DCF77_BIT_DURATION, UL_DCF77_BIT_DISTANCE);
					UC_DCF77_FIELD_CNT++;
				} //end of if (UC_DCF77_STATE == D_dcf77_state_SYNC_FOUND) && (UC_DCF77_FIELD_CNT <= (D_dcf77_MAX_NO_OF_BITS+1)
				if (UC_DCF77_FIELD_CNT == D_dcf77_MAX_NO_OF_BITS)
				{
					UC_DCF77_STATE = D_dcf77_state_FRAME_RECEIVED;
					if (UC_DCF77_RECEIVED_FRAME < 25)
					{
						UC_DCF77_RECEIVED_FRAME++;
					} //end of if (UC_DCF77_RECEIVED_FRAME < 25)
				} //end of if (UC_DCF77_FIELD_CNT == D_dcf77_MAX_NO_OF_BITS)
			} //end of else of if (UC_DCF77_FIELD_CNT == D_dcf77_state_NO_SIGNAL)
	   } //end of rising edge
	} //end of if (UC_DCF77_STATE < D_dcf77_state_FRAME_RECEIVED)
	asm("FSET I"); //enable all IRQ's
}
