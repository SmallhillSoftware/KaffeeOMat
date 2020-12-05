/************************************************************************************
*                                                                                   *
*   File Name   : globals.h                                                         *
*   Contents    : Global Variables for Funkuhr                                      *
*   Version     : 1.7                                                               *
*************************************************************************************/ 
#include "sfr_r813.h"                       /* Definition of the R8C/10 SFR */

//debugging defines
//#define UART1_DEBUGGING

//global defines
#define D_undefState      0
#define D_revDisplayState 1
#define D_adjustTimeState 2
#define D_runningState    3
#define D_warnVbattState  6

#define D_SHOW_REV_TIME_MS 3000
#define D_CONV_AD_TO_MV 20 //in real 20000/1024=19.53
#define D_TRUE  1
#define D_FALSE 0
#define D_NUM_OF_CHARS_PER_LINE 16


#define D_switchDebounceTimeMs 10

//analogue_inputs.c
#define d_undefState 0
#define d_startAN0 1
#define d_waitAN0  2
#define d_startAN1 3
#define d_waitAN1  4
#define d_startAN2 5
#define d_waitAN2  6
#define d_startAN3 7
#define d_waitAN3  8
#define ActLED_port           p3_3
#define ActLED_port_dir       pd3_3
#define EncoderUp_port        p4_5
#define EncoderUp_port_dir    pd4_5
#define EncoderDwn_port       p1_7
#define EncoderDwn_port_dir   pd1_7
#define EncoderOk_port        p3_2
#define EncoderOk_port_dir    pd3_2

//global variables
extern volatile unsigned long UL_TIRQ_count1ms;
extern volatile unsigned int  UI_STATE;
extern volatile unsigned int  UI_CLOCK_HOURS;
extern volatile unsigned int  UI_CLOCK_MINS;
extern volatile unsigned int  UI_CLOCK_SECS;
extern unsigned long          UL_int2_IRQ_timestamp;
extern unsigned int           UI_ENC_VALUE;
extern unsigned int           UI_VBAT_VOLTAGE_AD;
extern volatile char          C_ENC_DELTA;
extern _Bool                  B_SWITCH_OK;

//global support functions
unsigned long timerDifference (unsigned long OLD, unsigned long NEW);
unsigned int positiveDifference (unsigned int A, unsigned int B);

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
void f_vd_UART1_init(void);
void f_vd_sendValues(void);
