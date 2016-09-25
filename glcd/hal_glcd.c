#include "hal_glcd.h"

#include "../utils/utils.h"

#define CTRLRB (0x00)
#define CTRLR1 (0x01)
#define CTRLR2 (0x02)

#define WRITE (0x00)
#define READ (0x01)

#define INSTRUCTION (0x00)
#define DATA (0x01)

#define CS1 PE2
#define CS2 PE3
#define RS PE4
#define RW PE5
#define E PE6
#define RST PE7

#define EMASK 0xFD

#define D0 PA0
#define D1 PA1
#define D2 PA2
#define D3 PA3
#define D4 PA4
#define D5 PA5
#define D6 PA6
#define D7 PA7

#define DATABUS PORTA
#define ON_CMD 0x3F // 0011 1111

// convert coordinates into the byte format required by the coordinate setting functions
#define y_data(y) ((0x40 | (0x3F & y))) // (0100 0000 | (0011 1111 & y))
#define x_data(x) ((0xB8 | (0x07 & x))) // (1011 1000 | (0000 0111 & x))
#define z_data(z) ((0xC0 | (0x3F & z)))

// currently active controller
static uint8_t currCtrlr;

// controller space, not device space
static uint8_t xDeviceSpace;
static uint8_t yDeviceSpace;

// delay loops using inline assembler
static void nops3(void);
static void nops8(void);

// increments the saved Y position (yDeviceSpace),
// and wraps if the number has gone to the next controller
static void updateY(void);

// bit modification helper functions
static void halGlcdSetResetBit(void);
static void halGlcdSetEnableBit(void);
static void halGlcdClearEnableBit(void);
static void setRW(uint8_t rw);
static void setRS(uint8_t rs);

// selects the current controller
static void halGlcdCtrlSelect(const uint8_t controller);
// writes a command to one controller of the glcd
static void halGlcdCtrlWriteCmd(const uint8_t controller, const uint8_t data);
// writes data to one controller of the glcd
static void halGlcdCtrlWriteData(const uint8_t controller, const uint8_t data);
// reads data from one controller of the glcd
static uint8_t halGlcdCtrlReadData(const uint8_t controller);
// sets current address of one controller of the glcd
static void halGlcdCtrlSetAddress(const uint8_t controller, 
					const uint8_t x, const uint8_t y);
// loops until the glcd can be sent to
static void halGlcdCtrlBusyWait(const uint8_t controller);



/********************
 *
 * IMPLEMENTATIONS
 *
 ********************/
 
void halGlcdInit(void)
{
	setAsOut(&PORTE, &DDRE, EMASK);
	setAsOut(&PORTA, &DDRA, 0xFF);
	
	halGlcdSetResetBit();
	
	halGlcdCtrlWriteCmd(CTRLRB, ON_CMD);
}

uint8_t halGlcdSetAddress(const uint8_t xCol, const uint8_t yPage)
{
	uint8_t ctrlr = ((xCol < 64) ? CTRLR2 : CTRLR1);
	xDeviceSpace = yPage;
	yDeviceSpace = xCol % 64;
	
	halGlcdCtrlSetAddress(ctrlr, xDeviceSpace, yDeviceSpace);
	return 1;
}

void halGlcdSetYShift(uint8_t y)
{
	uint8_t oldCtrlr = currCtrlr;
	halGlcdCtrlWriteCmd(CTRLRB, z_data(y)); 
	currCtrlr = oldCtrlr;
}

uint8_t halGlcdWriteData(const uint8_t data)
{
	halGlcdCtrlWriteData(currCtrlr, data);
	updateY();
	return 0;
}

uint8_t halGlcdReadData(void)
{
	uint8_t res = halGlcdCtrlReadData(currCtrlr);
	updateY();
	return res;
}

/*****************
 *
 * HELPER FUNCTIONS
 *
 ****************/

static void nops3(void)
{
	uint32_t i;
	for(i = 0; i < 3; ++i) {
		asm volatile("nop\n"::);
	}
}

static void nops8(void)
{
	uint32_t i;
	for(i = 0; i < 8; ++i) {
		asm volatile("nop\n"::);
	}
}

static void updateY(void)
{
	++yDeviceSpace;
	if(yDeviceSpace == 64) {
		currCtrlr ^= 0x03; // swap last two bits
		yDeviceSpace = 0;
		halGlcdCtrlSetAddress(currCtrlr, xDeviceSpace, yDeviceSpace);
	}
}

static void halGlcdSetResetBit(void)
{
	nops3();
	set_port_bits(PORTE, (1 << PE7));
	nops8();
}

static void halGlcdSetEnableBit(void)
{
	nops8();
	set_port_bits(PORTE, (1 << PE6));
	nops8();
}

static void halGlcdClearEnableBit(void)
{
	clear_port_bits(PORTE, (1 << PE6));
}

static void setRW(uint8_t rw)
{
	clear_port_bits(PORTE, (1 << RW));
	set_port_bits(PORTE, ((rw & 0x01) << RW));
}

static void setRS(uint8_t rs)
{
	clear_port_bits(PORTE, (1 << RS));
	set_port_bits(PORTE, ((rs & 0x01) << RS));
}

static void halGlcdCtrlSelect(const uint8_t controller)
{
	clear_port_bits(PORTE, (1 << CS1) | (1 << CS2));
	set_port_bits(PORTE, ((controller & 0x01) << CS1));
	set_port_bits(PORTE, (((controller >> 1) & 0x01) << CS2));
	currCtrlr = (controller & 0x03);
}

static void halGlcdCtrlSetAddress(const uint8_t controller, 
					const uint8_t x, const uint8_t y)
{
	halGlcdCtrlWriteCmd(controller, x_data(x));
	halGlcdCtrlWriteCmd(controller, y_data(y));
}

static void halGlcdCtrlWriteCmd(const uint8_t controller, const uint8_t data)
{
	halGlcdCtrlBusyWait(controller);
	halGlcdCtrlSelect(controller);
	setRS(INSTRUCTION);
	setRW(WRITE);
	clear_port_bits(PORTA, 0xFF);
	set_port_bits(PORTA, data);
	
	halGlcdSetEnableBit();
	halGlcdClearEnableBit();
}

static void halGlcdCtrlWriteData(const uint8_t controller, const uint8_t data)
{
	halGlcdCtrlBusyWait(controller);
	halGlcdCtrlSelect(controller);
	setRS(DATA);
	setRW(WRITE);
	clear_port_bits(PORTA, 0xFF);
	set_port_bits(PORTA, data);
	
	halGlcdSetEnableBit();
	halGlcdClearEnableBit();
}

static uint8_t halGlcdCtrlReadData(const uint8_t controller)
{
	halGlcdCtrlBusyWait(controller);
	halGlcdCtrlSelect(controller);
	setRS(DATA);
	setRW(READ);
	setAsInNoPullUp(&PORTA, &DDRA, 0xFF);
	
	// dummy read
	halGlcdSetEnableBit();
	halGlcdClearEnableBit();
	
	// real read
	halGlcdSetEnableBit();
	uint8_t readVal = PINA;
	setAsOut(&PORTA, &DDRA, 0xFF);
	halGlcdClearEnableBit();
	
	return readVal;
}

static void halGlcdCtrlBusyWait(const uint8_t controller)
{
	halGlcdCtrlSelect(controller);
	//setAsInNoPullUp(&PORTA, &DDRA, 0xFF);
	
	uint8_t isBusy = 0xFF;
	while(isBusy)
	{
		PORTE |= (1 << RW);
		PORTE &= ~(1 << RS);
		PORTA = DDRA = 0x00;
		halGlcdSetEnableBit();
		isBusy = PINA & ((1 << D7) | (1 << D4)) ;
		halGlcdClearEnableBit();
	}
	setAsOut(&PORTA, &DDRA, 0xFF);
}
