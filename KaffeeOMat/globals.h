/************************************************************************************
*                                                                                   *
*   File Name   : globals.h                                                         *
*   Contents    : Global Variables for KaffeeOMat                                   *
*   Version     : 1.13, bases on global.h from Funkuhr 20201205 and 1.11 from MatchDisplay 20211026 *
*************************************************************************************/ 
#include "sfr_r813.h"                       /* Definition of the R8C/10 SFR */

//debugging defines
//#define UART1_DEBUGGING

//global defines
#define D_undefState      0
#define D_revDisplayState 1
#define D_adjustTimeState 2
#define D_runningState    3
#define D_setKaffeeTime   6
#define D_warnVbattState  7

#define D_SHOW_REV_TIME_MS 5000
#define D_WARNLEVEL_VBAT_MV 6700
/* valid for a voltage divider of 6k8 between measured voltage and ADC-input and 5k1 between ADC and GND */
#define D_CONV_ADC_2_VBAT_MV(x) ((unsigned int)(((unsigned long)114*(unsigned long)x)/((unsigned long)10)))
/* valid to ensure 5700mV with PWM-DC and considering 50mV VCE over transistor at 30mA */
#define D_CONV_VBAT_MV_2_DC(x) ((x>5750) ? ((unsigned char)(((unsigned long)143750)/((unsigned long)(x)))) : (25))
#define D_TRUE  1
#define D_FALSE 0
#define D_NUM_OF_CHARS_PER_LINE 16


#define D_switchDebounceTimeMs 10

//analogue_inputs.c
#define d_undefState 0
#define d_startAN11  1
#define d_waitAN11   2
#define ActLED_port           p3_3
#define ActLED_port_dir       pd3_3
#define BacklightMod_port     p3_0
#define BacklightMod_port_dir pd3_0
#define EncoderUp_port        p4_5
#define EncoderUp_port_dir    pd4_5
#define EncoderDwn_port       p1_7
#define EncoderDwn_port_dir   pd1_7
#define EncoderOk_port        p3_2
#define EncoderOk_port_dir    pd3_2

//global variables
extern volatile unsigned long UL_TIRQ_count1ms;
extern volatile unsigned char UC_TIRQ_pwmCount;
extern volatile unsigned int  UI_STATE;
extern volatile unsigned int  UI_CLOCK_DAYS;
extern volatile unsigned int  UI_CLOCK_HOURS;
extern volatile unsigned int  UI_CLOCK_MINS;
extern volatile unsigned int  UI_CLOCK_SECS;
extern unsigned long          UL_int2_IRQ_timestamp;
extern unsigned int           UI_ENC_VALUE;
extern volatile unsigned int  UI_VBAT_VOLTAGE_MV;
extern volatile char          C_ENC_DELTA;
extern _Bool                  B_SWITCH_OK;
extern _Bool                  B_VBAT_ACQUIRED;
extern _Bool                  B_BACKLIGHT_ON;
extern volatile unsigned char UC_BACKLIGHT_DUTY;


void f_vd_controllerInit(void);
void f_vd_outputHandler(unsigned long content_to_display);
void f_vd_TIMERX_init(void);
void f_vd_TIMERY_init(void);
void f_vd_TIMERZ_init(void);
void f_vd_EXTINTS_init(void);
void f_vd_adcInit(void);
void f_vd_AcquireVoltages(void);
void f_vd_ENCODER_READ(void);

//debug
#ifdef UART1_DEBUGGING
void f_vd_UART1_init(void);
void f_vd_sendValues(void);
#endif
