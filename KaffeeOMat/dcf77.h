/************************************************************************************
*                                                                                   *
*   File Name   : dcf77.h                                                           *
*   Contents    : Decoder of DCF77-signal for Funkuhr                               *
*   Version     : 1.22                                                              *
*************************************************************************************/ 
#define DATE_REQUIRED 1

//states of UC_DCF77_STATE
#define D_dcf77_state_NO_SIGNAL        0
#define D_dcf77_state_BIT_RECEIVED     1
#define D_dcf77_state_SYNC_FOUND       2
#define D_dcf77_state_FRAME_RECEIVED   3
#define D_dcf77_state_FRAME_CHECKED    4
#define D_dcf77_state_DATE_VALID       5
//further needed states
#define D_dcf77_state_maxValue       255

//return values of frame validity check
#define D_dcf77_valid_reception        0
#define D_dcf77_invalid_bit_distance   1
#define D_dcf77_invalid_frame_start    2
#define D_dcf77_invalid_mez_mesz       3
#define D_dcf77_invalid_time_start     4
#define D_dcf77_invalid_minute_parity  5
#define D_dcf77_invalid_hour_parity    6
#define D_dcf77_invalid_date_parity    7
#define D_dcf77_invalid_minute_val     8
#define D_dcf77_invalid_hour_val       9
#define D_dcf77_invalid_day_val       10
#define D_dcf77_invalid_month_val     11
#define D_dcf77_invalid_year_val      12
#define D_dcf77_25_wrong_frames       13
#define D_dcf77_frame_eval_too_early  14
#define D_dcf77_less_than_3_min_vals  15
#define D_dcf77_less_than_3_hr_vals   16
#define D_dcf77_less_than_3_dy_vals   17
#define D_dcf77_less_than_3_mn_vals   18
#define D_dcf77_less_than_3_yr_vals   19
#define D_dcf77_3_min_vals_not_match  20
#define D_dcf77_3_hr_vals_not_match   21
#define D_dcf77_3_dy_vals_not_match   22
#define D_dcf77_3_mn_vals_not_match   23
#define D_dcf77_3_yr_vals_not_match   24
//further needed invalid causes
#define D_dcf77_invalid_maxValue     255

//return values of date validity check
#define D_date_valid                   0
#define D_date_invalid                 1
#define D_date_eval_too_early          2

/*values of UC_DCF77_EDGE, defined polarity should match with needed IRQ-polarity in
* processor register, next define has to be set (1) for 5V-supplied Reichelt-DCF77-
* receiver with no level shifter, meaning no inverter, but has to reset (0) for 3,3V-
* or 1,8V-supplied Pollin-DCF77-receiver with level shifter that inverts the signal. 
*/
#define D_reicheltDCF77_noInverter     1
#if D_reicheltDCF77_noInverter
	#define D_dcf77_FALLING_EDGE           1
	#define D_dcf77_RISING_EDGE            0
#else
	#define D_dcf77_FALLING_EDGE           0
	#define D_dcf77_RISING_EDGE            1
#endif


#define D_dcf77_INPUT_PIN           p1_0
#define D_dcf77_INPUT_PIN_DIR      pd1_0
#define D_dcf77_PONOUT_PIN          p1_1
#define D_dcf77_PONOUT_PIN_DIR     pd1_1

#define D_dcf77_IRQ_SRC_ENABLE     ki0en
#define D_dcf77_IRQ_POLARITY       ki0pl
#define D_dcf77_IRQ_CTRL           kupic
#define D_dcf77_IRQ_REQ         ir_kupic

#define D_dcf77_SYNC_PULSE_LENGTH    2000 //2000ms became 1941 counts, standard distance is 971 counts
#define D_dcf77_SYNC_PULSE_TOLERANCE  300 //300ms
#define D_dcf77_LOW_PULSE_LO_LIMIT     80 //100ms nominal
#define D_dcf77_LOW_PULSE_HI_LIMIT    180
#define D_dcf77_SHORTEST_BIT_DISTANCE 700 //1000ms nominal
#define D_dcf77_MAX_NO_OF_BITS         59
#ifdef DATE_REQUIRED
	#define D_dcf77_MAX_NO_OF_USED_BITS    58
	#define D_dcf77_YEAR_OFFSET          2000
#else
	#define D_dcf77_MAX_NO_OF_USED_BITS    35
#endif

struct ST_MINVALS
{
	unsigned int  uc_DATEVAL         : 7;
	unsigned int  bt_DATEVAL_VALID   : 1;
	unsigned int  uc_DATEVAL_FRAMENO : 8;
	unsigned int  uc_noOfTmpDateVals : 3;
	unsigned int  uc_DATEVAL_ptyValid: 2;
	unsigned int  empty              :11;
	unsigned char uc_tmpDATEVAL[3];
	unsigned long ul_tmpDV_timestamp[3];
};

struct ST_DATEVALS
{
	unsigned int  uc_DATEVAL         : 7;
	unsigned int  bt_DATEVAL_VALID   : 1;
	unsigned int  uc_DATEVAL_FRAMENO : 8;
	unsigned int  uc_noOfTmpDateVals : 3;
	unsigned int  uc_DATEVAL_ptyValid: 2;
	unsigned int  empty              :11;
	unsigned char uc_tmpDATEVAL[3];
};

//debug only
extern volatile unsigned char UC_DCF77_RECEIVED_FRAME;
extern volatile unsigned char UC_DCF77_STATE;
extern volatile unsigned char UC_DCF77_EDGE;
extern volatile unsigned long UL_DCF77_FALLING_TIME;
extern volatile unsigned long UL_DCF77_PREVIOUS_FALLING_TIME;
extern volatile unsigned long UL_DCF77_RISING_TIME;
extern volatile unsigned long UL_DCF77_BIT_DURATION;
extern volatile unsigned long UL_DCF77_BIT_DISTANCE;
extern volatile unsigned char UC_DCF77_FIELD_CNT;
extern volatile unsigned char UC_DCF77_BIT_FIELD[D_dcf77_MAX_NO_OF_BITS+1];
extern unsigned char UC_DCF77_DATE_BITCOUNT;
extern unsigned char UC_DCF77_HOUR_BITCOUNT;
extern unsigned char UC_DCF77_MIN_BITCOUNT;
extern volatile struct ST_MINVALS    ST_MINUTE;
extern volatile struct ST_DATEVALS   ST_HOUR;
extern unsigned char UC_DATE_MINUTE;
extern unsigned char UC_DATE_HOUR;
#ifdef DATE_REQUIRED
extern struct ST_DATEVALS   ST_DAY;
extern struct ST_DATEVALS   ST_MONTH;
extern struct ST_DATEVALS   ST_YEAR;
extern unsigned char UC_DATE_DAY;
extern unsigned char UC_DATE_MONTH;
extern unsigned int  UI_DATE_YEAR;
#endif

unsigned char f_uc_check_dcf77_date_validity(void);
unsigned char f_uc_check_dcf77_frame_validity(unsigned int ui_secs, unsigned int ui_mins, unsigned int ui_hours);
unsigned char f_uc_get_dcf77_state(void);
void          f_vd_DCF77_init(_Bool dcf77_tmp_data_reset);
void          f_vd_DCF77_disableIRQ(void);
unsigned char f_uc_dcf77_get_date_minute(void);
unsigned char f_uc_dcf77_get_date_hour(void);
#ifdef DATE_REQUIRED
unsigned char f_uc_dcf77_get_date_day(void);
unsigned char f_uc_dcf77_get_date_month(void);
#endif