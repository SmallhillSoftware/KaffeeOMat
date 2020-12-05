/***************************************************************************************
*                                                                                      *
*   File Name   : ST7920.c                                                             *
*   Contents    : Display driver for Pollin DG14032 adapted to R8C13 (Doppelkopftimer) *
*   Version     : 1.13                                                                 *
*                                                                                      *
****************************************************************************************/ 
#include "globals.h"

// save program memory -> we do not use _delay_us()
// 1 nop per MHz (adapt if needed)
#define DELAY_1US \
	{			\
	asm("nop"); \
	asm("nop"); \
	asm("nop"); \
	asm("nop"); \
	asm("nop"); \
	asm("nop"); \
	asm("nop"); \
	asm("nop"); \
	}



#define CMD_DISPLAY_CLEAR	0x01
#define CMD_RETURN_HOME		0x02
#define CMD_ENTRY_MODE		0x04
#define CMD_DISPLAY_CONTROL	0x08
#define CMD_CURSOR_CONTROL	0x10
#define CMD_FUNCTION_SET	0x20
#define CMD_SET_CGRAM_ADR	0x40
#define CMD_SET_DDRAM_ADR	0x80

#define CMDE_STANDBY		0x01
#define CMDE_SCROLL_SEL		0x02
#define CMDE_REVERSE_LINE	0x04
#define CMDE_SET_SCROLL_ADR	0x40
#define CMDE_SET_GDRAM_ADR	0x80

#define BIT_ID 				0x02
#define BIT_S 				0x01
#define BIT_D				0x04
#define BIT_C				0x02
#define BIT_B				0x01
#define BIT_SC				0x08
#define BIT_SR				0x01
#define BIT_RL				0x04
#define BIT_DL				0x10
#define BIT_RE				0x04
#define BIT_G				0x02
#define BIT_BUSY			0x80

#define MASK_CGRAM_ADR		0x3F
#define MASK_DDRAM_ADR		0x3F
#define MASK_SCROLL_ADR		0x3F
#define MASK_GDRAM_ADR_HOR	0x0F
#define MASK_GDRAM_ADR_VER	0x3F

#define PIN_OUTPUT_DEF          1    //1 = configures output, 0 = configures input

#define D4_PIN_DIR_REG   	pd0_4
#define D4_PIN_PORT_REG   	p0_4
#if (PIN_OUTPUT_DEF == 1)
    #define CONF_D4_PIN_OUTPUT\
    {\
     prc2 = 1;\
     D4_PIN_DIR_REG = 1;\
    }
	#define CONF_D4_PIN_INPUT\
    {\
     prc2 = 1;\
     D4_PIN_DIR_REG = 0;\
    }
#else
    #define CONF_D4_PIN_OUTPUT\
    {\
     prc2 = 1;\
     D4_PIN_DIR_REG = 0;\
    }
	#define CONF_D4_PIN_INPUT\
    {\
     prc2 = 1;\
     D4_PIN_DIR_REG = 1;\
    }
#endif

#define D5_PIN_DIR_REG   	pd0_5
#define D5_PIN_PORT_REG   	p0_5
#if (PIN_OUTPUT_DEF == 1)
    #define CONF_D5_PIN_OUTPUT\
    {\
     prc2 = 1;\
     D5_PIN_DIR_REG = 1;\
    }
	#define CONF_D5_PIN_INPUT\
    {\
     prc2 = 1;\
     D5_PIN_DIR_REG = 0;\
    }
#else
    #define CONF_D5_PIN_OUTPUT\
    {\
     prc2 = 1;\
     D5_PIN_DIR_REG = 0;\
    }
	#define CONF_D5_PIN_INPUT\
    {\
     prc2 = 1;\
     D5_PIN_DIR_REG = 1;\
    }
#endif

#define D6_PIN_DIR_REG   	pd0_6
#define D6_PIN_PORT_REG   	p0_6
#if (PIN_OUTPUT_DEF == 1)
    #define CONF_D6_PIN_OUTPUT\
    {\
     prc2 = 1;\
     D6_PIN_DIR_REG = 1;\
    }
	#define CONF_D6_PIN_INPUT\
    {\
     prc2 = 1;\
     D6_PIN_DIR_REG = 0;\
    }
#else
    #define CONF_D6_PIN_OUTPUT\
    {\
     prc2 = 1;\
     D6_PIN_DIR_REG = 0;\
    }
	#define CONF_D6_PIN_INPUT\
    {\
     prc2 = 1;\
     D6_PIN_DIR_REG = 1;\
    }
#endif

#define D7_PIN_DIR_REG   	pd0_7
#define D7_PIN_PORT_REG   	p0_7
#if (PIN_OUTPUT_DEF == 1)
    #define CONF_D7_PIN_OUTPUT\
    {\
     prc2 = 1;\
     D7_PIN_DIR_REG = 1;\
    }
	#define CONF_D7_PIN_INPUT\
    {\
     prc2 = 1;\
     D7_PIN_DIR_REG = 0;\
    }
#else
    #define CONF_D7_PIN_OUTPUT\
    {\
     prc2 = 1;\
     D7_PIN_DIR_REG = 0;\
    }
	#define CONF_D7_PIN_INPUT\
    {\
     prc2 = 1;\
     D7_PIN_DIR_REG = 1;\
    }
#endif

#define E_PIN_DIR_REG   	pd0_3
#define E_PIN_PORT_REG   	p0_3
#if (PIN_OUTPUT_DEF == 1)
    #define CONF_E_PIN_OUTPUT\
    {\
     prc2 = 1;\
     E_PIN_DIR_REG = 1;\
    }
#else
    #define CONF_E_PIN_OUTPUT\
    {\
     prc2 = 1;\
     E_PIN_DIR_REG = 0;\
    }
#endif

#define RS_PIN_DIR_REG  	pd0_2
#define RS_PIN_PORT_REG  	p0_2
#if (PIN_OUTPUT_DEF == 1)
    #define CONF_RS_PIN_OUTPUT\
    {\
     prc2 = 1;\
     RS_PIN_DIR_REG = 1;\
    }
#else
    #define CONF_RS_PIN_OUTPUT\
    {\
     prc2 = 1;\
     RS_PIN_DIR_REG = 0;\
    }
#endif

#define RW_PIN_DIR_REG   	pd0_1
#define RW_PIN_PORT_REG   	p0_1
#if (PIN_OUTPUT_DEF == 1)
    #define CONF_RW_PIN_OUTPUT\
    {\
     prc2 = 1;\
     RW_PIN_DIR_REG = 1;\
    }
#else
    #define CONF_RW_PIN_OUTPUT\
    {\
     prc2 = 1;\
     RW_PIN_DIR_REG = 0;\
    }
#endif


#define RS_DATA		1
#define RS_CMD		0

#define BIT_SET         1
#define BIT_RESET       0
#define BIT_OUTPUT      1
#define BIT_INPUT       0

#define EVEN   			1
#define UNEVEN 			2

static unsigned char G_ExtendedMode = 0;

static char dispContent[2][D_NUM_OF_CHARS_PER_LINE+1];

unsigned int checkEven_Uneven(unsigned int numberToCheck)
{
	if(numberToCheck%2)
	{
		return UNEVEN;
	}
	else
	{
		return EVEN;
	}
}

unsigned char calcDDRAMaddress(unsigned char x, unsigned char y)
{
unsigned char addr;
	if (y > 1)
	{
		y = 1;
	}
	if (x > 15)
	{
		x = 15;
	}
	addr = (x >> 1) + CMD_SET_DDRAM_ADR + (y << 4);
	return addr;
}


static void RS (unsigned char val) {
	if (val) RS_PIN_PORT_REG = BIT_SET; else RS_PIN_PORT_REG = BIT_RESET;
	}

static void E (unsigned char val) {
	if (val) E_PIN_PORT_REG = BIT_SET;  else E_PIN_PORT_REG = BIT_RESET;
	}

static void RW (unsigned char val) {
	if (val) RW_PIN_PORT_REG = BIT_SET; else RW_PIN_PORT_REG = BIT_RESET;
	}

void Delay (unsigned long dval) {
	while (dval--) DELAY_1US;
}

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------

void SetDataOutput (void) {

	CONF_D4_PIN_OUTPUT;
	CONF_D5_PIN_OUTPUT;
	CONF_D6_PIN_OUTPUT;
	CONF_D7_PIN_OUTPUT;

}

void SetDataInput (void) {

	CONF_D4_PIN_INPUT;
	CONF_D5_PIN_INPUT;
	CONF_D6_PIN_INPUT;
	CONF_D7_PIN_INPUT;

}

void GLCD_InitPorts(void)
{
	//Output
	CONF_E_PIN_OUTPUT;
	CONF_RS_PIN_OUTPUT;
	CONF_RW_PIN_OUTPUT;
        RS(0);
	RW(1);
	E(0);
	//Input
	SetDataInput();
}


void DataOut (unsigned char data) {

	SetDataOutput();

	if (data&0x01) D4_PIN_PORT_REG = BIT_SET; else D4_PIN_PORT_REG = BIT_RESET;
	if (data&0x02) D5_PIN_PORT_REG = BIT_SET; else D5_PIN_PORT_REG = BIT_RESET;
	if (data&0x04) D6_PIN_PORT_REG = BIT_SET; else D6_PIN_PORT_REG = BIT_RESET;
	if (data&0x08) D7_PIN_PORT_REG = BIT_SET; else D7_PIN_PORT_REG = BIT_RESET;
	//SetDataOutput(); //orig code
}


unsigned char DataIn (void) {

	unsigned char data=0;

	SetDataInput();

	data = (D4_PIN_PORT_REG & 1);
        data = data + ((D5_PIN_PORT_REG & 1)<<1);
        data = data + ((D6_PIN_PORT_REG & 1)<<2);
	data = data + ((D7_PIN_PORT_REG & 1)<<3);
	return(data);
}

//-------------------------------------------------------------------------------------------------
// Read
//-------------------------------------------------------------------------------------------------
unsigned char GLCD_rawRead(unsigned char regsel)
{
	unsigned char tmp;

	E(0);
	SetDataInput();
	RS(regsel);
	RW(1);
	Delay(1);
	E(1);
	Delay(1);
	tmp = DataIn();
	E(0);
	Delay(1);
	E(1);
	Delay(1);
	tmp <<= 4;
	tmp |= DataIn();
	E(0);
	Delay(1);

	return tmp;
}

//-------------------------------------------------------------------------------------------------
// Wait Busy
//-------------------------------------------------------------------------------------------------
void GLCD_WaitBusy(void)
{
	while(GLCD_rawRead(0)&BIT_BUSY);
}

unsigned char GLCD_Read(unsigned char regsel)
{
	unsigned char tmp;
        tmp = GLCD_rawRead(regsel);
        GLCD_WaitBusy();

        return tmp;
}

//-------------------------------------------------------------------------------------------------
// Write 
//-------------------------------------------------------------------------------------------------
static void GLCD_Write(unsigned char regsel, unsigned char val)
{
	SetDataOutput();
	RS(regsel);
        RW(0);
	DataOut(val>>4); //upper nibble
	Delay(1);
	E(1);
	Delay(1);
	E(0);
	DataOut(val&0xF); //lower nibble
	Delay(1);
	E(1);
	Delay(1);
	E(0);
    //RW(1); //Wozu ?
	//RS(1); //Wozu ?
	//SetDataInput();

	GLCD_WaitBusy();

}
//-------------------------------------------------------------------------------------------------
// BasicMode
//-------------------------------------------------------------------------------------------------
void GLCD_SetBasicMode(void)
{
	if (G_ExtendedMode) {
		GLCD_Write(RS_CMD,CMD_FUNCTION_SET);
		G_ExtendedMode = 0;
		}
}

//-------------------------------------------------------------------------------------------------
// ExtendedMode
//-------------------------------------------------------------------------------------------------
void GLCD_SetExtendedMode(void)
{
	if (!G_ExtendedMode) 
		{
		GLCD_Write(RS_CMD,CMD_FUNCTION_SET|BIT_RE|BIT_G);
		G_ExtendedMode = 1;
		}
}

//-------------------------------------------------------------------------------------------------
// ClearTXT
//-------------------------------------------------------------------------------------------------
void GLCD_ClearTXT(void)
{
	GLCD_SetBasicMode();
	GLCD_Write(RS_CMD,CMD_DISPLAY_CLEAR);
	GLCD_WaitBusy();

}

//-------------------------------------------------------------------------------------------------
// ClearGFX
//-------------------------------------------------------------------------------------------------
void GLCD_ClearGFX(void)
{
	unsigned char x,y;
	
	GLCD_SetExtendedMode();
	GLCD_Write(RS_CMD,CMDE_SET_GDRAM_ADR);
	GLCD_Write(RS_CMD,CMDE_SET_GDRAM_ADR);

	for (y=0;y<64;y++) {
		GLCD_Write(RS_CMD,CMDE_SET_GDRAM_ADR|(MASK_GDRAM_ADR_VER&y));
		GLCD_Write(RS_CMD,CMDE_SET_GDRAM_ADR);
		for (x=0;x<32;x++) GLCD_Write(RS_DATA,0x0);
		}
}

//-------------------------------------------------------------------------------------------------
// SetPixel
//-------------------------------------------------------------------------------------------------
void GLCD_SetPixel(unsigned char x, unsigned char y, unsigned char color)
{
	unsigned char subpos;
	unsigned char pix;
	unsigned char pixel_low;
	unsigned char pixel_high;

	GLCD_SetExtendedMode();
	GLCD_Write(RS_CMD,CMDE_SET_GDRAM_ADR|(MASK_GDRAM_ADR_VER&y));
	GLCD_Write(RS_CMD,CMDE_SET_GDRAM_ADR|(MASK_GDRAM_ADR_HOR&(x>>4)));
	GLCD_Read(RS_DATA);
	pixel_high  = GLCD_Read(RS_DATA);
	pixel_low   = GLCD_Read(RS_DATA);

	subpos = x & 0x0F;
	GLCD_Write(RS_CMD,CMDE_SET_GDRAM_ADR|(MASK_GDRAM_ADR_VER&y));
	GLCD_Write(RS_CMD,CMDE_SET_GDRAM_ADR|(MASK_GDRAM_ADR_HOR&(x>>4)));
	
	if (subpos < 8) {
		pix = color ? (1<<(7-subpos))|pixel_high : (~(1<<(7-subpos)))&pixel_high;
		GLCD_Write(RS_DATA,pix);
		GLCD_Write(RS_DATA,pixel_low);
		}
	else {
		pix = color ? (1<<(15-subpos))|pixel_low : (~(1<<(7-subpos)))&pixel_low;
		GLCD_Write(RS_DATA,pixel_high);
		GLCD_Write(RS_DATA,pix);
		}
	
}

//-------------------------------------------------------------------------------------------------
// Vertical Offset
//-------------------------------------------------------------------------------------------------
void GLCD_VerticalOffset(unsigned char offs)
{
	GLCD_SetExtendedMode();


	GLCD_Write(RS_CMD,CMDE_SCROLL_SEL|BIT_SR);
	GLCD_Write(RS_CMD,CMDE_SET_SCROLL_ADR|(MASK_SCROLL_ADR&offs));
	GLCD_Write(RS_CMD,CMDE_SCROLL_SEL);
	GLCD_WaitBusy();
}



//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void GLCD_WriteString(unsigned char x, unsigned char y, char * s)
{
unsigned int idx = 0;
unsigned int xLen = 0;
char char2Write;	
	while( (*s) && ((x + idx) < D_NUM_OF_CHARS_PER_LINE) )
	{
  		dispContent[y][x+idx] = *s;
		s++;
		idx++;
  	}
  	xLen = idx;
  	if (checkEven_Uneven(x) == UNEVEN)
	{
		x = x - 1;
		xLen = xLen + 1;
	}
	if (checkEven_Uneven(xLen) == UNEVEN)
	{
		xLen = xLen + 1;
	}
	GLCD_SetBasicMode();
	GLCD_Write(RS_CMD,calcDDRAMaddress(x,y));
	for(idx=0;idx<=xLen;idx++)
	{
		char2Write = dispContent[y][x+idx];
		GLCD_Write(RS_DATA,char2Write);
	}
}

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void GLCD_WriteUINumber(unsigned char x, unsigned char y, unsigned int z, _Bool preventLeadingZeros, unsigned char digitsToDisplay)
{
unsigned char digit;
unsigned int  value;	
unsigned char leadingZerosSuppressed = 0;
char char2Write;
unsigned int idx = 0;
unsigned int xLen = 0;

	if (digitsToDisplay > 5) digitsToDisplay = 5;
	value = z;
	switch (digitsToDisplay)
	{
		case 5:		
			digit = (unsigned char)(value/(unsigned int)10000);
			if (digit == 0)
			{
				if (!preventLeadingZeros)
				{
					dispContent[y][x+idx] = 0x30+digit;
					idx++;
				}
				else
				{
					leadingZerosSuppressed++;
				}
			}
			else
			{
				dispContent[y][x+idx] = 0x30+digit;
				idx++;
				leadingZerosSuppressed = 0;
			}
			value = z - ((unsigned int)digit*(unsigned int)10000);
		case 4:
			digit = (unsigned char)(value/(unsigned int)1000);
			if ((digit == 0) && (leadingZerosSuppressed > 0))
			{
				if (!preventLeadingZeros)
				{
					dispContent[y][x+idx] = 0x30+digit;
					idx++;
				}
				else
				{
					leadingZerosSuppressed++;
				}
			}
			else
			{
				dispContent[y][x+idx] = 0x30+digit;
				idx++;
				leadingZerosSuppressed = 0;
			}
			value = value - ((unsigned int)digit*(unsigned int)1000);
		case 3:
			digit = (unsigned char)(value/(unsigned int)100);
			if ((digit == 0) && (leadingZerosSuppressed > 0))
			{
				if (!preventLeadingZeros)
				{
					dispContent[y][x+idx] = 0x30+digit;
					idx++;
				}
				else
				{
					leadingZerosSuppressed++;
				}
			}
			else
			{
				dispContent[y][x+idx] = 0x30+digit;
				idx++;
				leadingZerosSuppressed = 0;
			}
			value = value - ((unsigned int)digit*(unsigned int)100);
		case 2:
			digit = (unsigned char)(value/(unsigned int)10);
			if ((digit == 0) && (leadingZerosSuppressed > 0))
			{
				if (!preventLeadingZeros)
				{
					dispContent[y][x+idx] = 0x30+digit;
					idx++;
				}
				else
				{
					leadingZerosSuppressed++;
				}
			}
			else
			{
				dispContent[y][x+idx] = 0x30+digit;
				idx++;
				leadingZerosSuppressed = 0;
			}
			value = value - ((unsigned int)digit*(unsigned int)10);
		case 1:
			dispContent[y][x+idx] = 0x30+value;
			idx++;
		case 0:
			break;
	} //end of switch (digitsToDisplay)
	xLen = digitsToDisplay;
	if (checkEven_Uneven(x) == UNEVEN)
	{
		x = x - 1;
		xLen = xLen + 1;
	}
	if (checkEven_Uneven(xLen) == UNEVEN)
	{
		xLen = xLen + 1;
	}
	GLCD_SetBasicMode();
	GLCD_Write(RS_CMD, calcDDRAMaddress(x,y));
	for(idx=0;idx<=xLen;idx++)
	{
		char2Write = dispContent[y][x+idx];
		GLCD_Write(RS_DATA,char2Write);
	}	
}

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void GLCD_WriteULNumber(unsigned char x, unsigned char y, unsigned long z, _Bool preventLeadingZeros)
{
unsigned char digit;
unsigned long value;	
unsigned char leadingZerosSuppressed = 0;
char char2Write;
unsigned int idx = 0;
unsigned int xLen = 0;

	digit = z/1000000000;
	if (digit == 0)
	{
		if (!preventLeadingZeros)
		{
			dispContent[y][x+idx] = 0x30+digit; //0
			idx++;
		}
		else
		{
			leadingZerosSuppressed++;
		}
	}
	else
	{
		dispContent[y][x+idx] = 0x30+digit; //0
		idx++;
		leadingZerosSuppressed = 0;
	}
	value = z - ((unsigned long)digit*1000000000);
	digit = value/100000000;
	if ((digit == 0) && (leadingZerosSuppressed > 0))
	{
		if (!preventLeadingZeros)
		{
			dispContent[y][x+idx] = 0x30+digit; //0
			idx++;
		}
		else
		{
			leadingZerosSuppressed++;
		}
	}
	else
	{
		dispContent[y][x+idx] = 0x30+digit; //0
		idx++;
		leadingZerosSuppressed = 0;
	}
	value = value - ((unsigned long)digit*100000000);
	digit = value/10000000;
	if ((digit == 0) && (leadingZerosSuppressed > 0))
	{
		if (!preventLeadingZeros)
		{
			dispContent[y][x+idx] = 0x30+digit; //2
			idx++;
		}
		else
		{
			leadingZerosSuppressed++;
		}
	}
	else
	{
		dispContent[y][x+idx] = 0x30+digit; //2
		idx++;
		leadingZerosSuppressed = 0;
	}
	value = value - ((unsigned long)digit*10000000);
	digit = value/1000000;
	if ((digit == 0) && (leadingZerosSuppressed > 0))
	{
		if (!preventLeadingZeros)
		{
			dispContent[y][x+idx] = 0x30+digit; //0
			idx++;
		}
		else
		{
			leadingZerosSuppressed++;
		}
	}
	else
	{
		dispContent[y][x+idx] = 0x30+digit; //0
		idx++;
		leadingZerosSuppressed = 0;
	}
	value = value - ((unsigned long)digit*1000000);
	digit = value/100000;
	if ((digit == 0) && (leadingZerosSuppressed > 0))
	{
		if (!preventLeadingZeros)
		{
			dispContent[y][x+idx] = 0x30+digit; //1
			idx++;
		}
		else
		{
			leadingZerosSuppressed++;
		}
	}
	else
	{
		dispContent[y][x+idx] = 0x30+digit; //1
		idx++;
		leadingZerosSuppressed = 0;
	}
	value = value - ((unsigned long)digit*100000);
	digit = value/10000;
	if ((digit == 0) && (leadingZerosSuppressed > 0))
	{
		if (!preventLeadingZeros)
		{
			dispContent[y][x+idx] = 0x30+digit; //5
			idx++;
		}
		else
		{
			leadingZerosSuppressed++;
		}
	}
	else
	{
		dispContent[y][x+idx] = 0x30+digit; //5
		idx++;
		leadingZerosSuppressed = 0;
	}
	value = value - ((unsigned long)digit*10000);
	digit = value/1000;
	if ((digit == 0) && (leadingZerosSuppressed > 0))
	{
		if (!preventLeadingZeros)
		{
			dispContent[y][x+idx] = 0x30+digit; //0
			idx++;
		}
		else
		{
			leadingZerosSuppressed++;
		}
	}
	else
	{
		dispContent[y][x+idx] = 0x30+digit; //0
		idx++;
		leadingZerosSuppressed = 0;
	}
	value = value - ((unsigned long)digit*1000);
	digit = value/100;
	if ((digit == 0) && (leadingZerosSuppressed > 0))
	{
		if (!preventLeadingZeros)
		{
			dispContent[y][x+idx] = 0x30+digit; //0
			idx++;
		}
		else
		{
			leadingZerosSuppressed++;
		}
	}
	else
	{
		dispContent[y][x+idx] = 0x30+digit; //0
		idx++;
		leadingZerosSuppressed = 0;
	}
	value = value - ((unsigned long)digit*100);
	digit = value/10;
	if ((digit == 0) && (leadingZerosSuppressed > 0))
	{
		if (!preventLeadingZeros)
		{
			dispContent[y][x+idx] = 0x30+digit; //0
			idx++;
		}
		else
		{
			leadingZerosSuppressed++;
		}
	}
	else
	{
		dispContent[y][x+idx] = 0x30+digit; //0
		idx++;
		leadingZerosSuppressed = 0;
	}
	value = value - ((unsigned long)digit*10);
	dispContent[y][x+idx] = 0x30+value; //0
	xLen = idx;
	GLCD_SetBasicMode();
	GLCD_Write(RS_CMD, calcDDRAMaddress(x,y));
	for(idx=0;idx<=xLen;idx++)
	{
		char2Write = dispContent[y][x+idx];
		GLCD_Write(RS_DATA,char2Write);
	}
}

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void GLCD_Init(void)
{
unsigned int i,j;

	//fill RAM-buffer in µC for display content with space-characters	
	for (j=0;j<2;j++)
	{	
		for(i=0;i<17;i++)
		{
			dispContent[j][i] = ' ';
		}
	}	
	GLCD_InitPorts();

	Delay(100000);

	GLCD_Write(RS_CMD,CMD_FUNCTION_SET);
	GLCD_Write(RS_CMD,CMD_FUNCTION_SET);
	GLCD_Write(RS_CMD,CMD_DISPLAY_CONTROL|BIT_D);
	Delay(100);
	GLCD_Write(RS_CMD,CMD_DISPLAY_CLEAR);
	Delay(100);
	GLCD_Write(RS_CMD,CMD_ENTRY_MODE|BIT_ID);
	Delay(100);
	G_ExtendedMode = 0;
}

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void GLCD_SetCursor(unsigned char x, unsigned char y)
{
unsigned char i;
	GLCD_SetBasicMode();
	//hier beachten, dass der Cursor nur auf jede gerade x-Position gesetzt werden kann!!
	GLCD_Write(RS_CMD,calcDDRAMaddress(x,y));
	GLCD_Write(RS_CMD,(CMD_DISPLAY_CONTROL|BIT_D|BIT_C)); //display on + cursor on
	/*
	GLCD_WaitBusy();
	Delay(100);
	for (i=0; i<uc_deltaPos; i++)
	{
	    if (b_moveRight)
	    {
	       GLCD_Write(RS_CMD,CMD_CURSOR_CONTROL|BIT_RL);
	    }
	    else
	    {
	       GLCD_Write(RS_CMD,CMD_CURSOR_CONTROL);
	    }   
        GLCD_WaitBusy();
            Delay(100);
	}
	*/    
}

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void GLCD_CursorOff(void)
{
	GLCD_Write(RS_CMD,(CMD_DISPLAY_CONTROL|BIT_D)); //display on + cursor off
	Delay(100);
}