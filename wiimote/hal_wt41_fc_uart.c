#include "hal_wt41_fc_uart.h" 

#include <assert.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "../utils/utils.h"

#define F_CPU   16000000UL
#define BAUD	1000000UL
#define UBRR_VAL ((F_CPU / (16 * BAUD)) - 1)

#define TXD3 PJ0
#define RXD3 PJ1
#define RTS PJ2
#define CTS PJ3

// ringbuffer capacity
#define RBF_CAP 64
#define rbf_init() do{ rbf.size = rbf.headIdx = rbf.tailIdx = 0; } while (0)

#define PRESCALAR (1 << CS31 | 1 << CS30)
#define TICKS 1250

// a minimalistic ringbuffer data structure
typedef struct ringbuffer_t {
	uint8_t buffer[RBF_CAP];
	uint8_t size;
	uint8_t headIdx;
	uint8_t tailIdx;
} ringbuffer;

// byte that is to be sent the next time the port is empty
static volatile uint8_t leftover;
// set if there is something in "leftover" to be sent
static volatile uint8_t sending = 0;
// set if we need to stop sending due to the RTS flag
static volatile uint8_t pauseSending = 0;

// set if there is one piece of code emptying the rbf
static volatile uint8_t dispatching = 0;

// callbacks for sending and recieving
static void (*_sndCallback)();
static void (*_rcvCallback)(uint8_t);

// enable and disable interrupts for when the port is empty
static void enableEmptyInterrupt(void);
static void disableEmptyInterrupt(void);

// enable and disable clear to send flag
static void enableCTS(void);
static void disableCTS(void);

// retrieves the RTS flag
static uint8_t getRTS(void);

// sets up initial timer
static void setupTimer(void);

static volatile ringbuffer rbf;

ISR (TIMER3_COMPA_vect)
{
	set_port_bits(PORTJ, (1 << PJ5));
	TIMSK3 &= ~(1 << OCIE3A);
	
	// enable transmit, receive and receive interrupt
	set_port_bits(UCSR3B, (1 << RXEN3) | (1 << TXEN3) | (1 << RXCIE3));
}

ISR(USART3_RX_vect, ISR_BLOCK)
{
	rbf.buffer[rbf.headIdx] = UDR3;
	rbf.headIdx = (rbf.headIdx + 1) % RBF_CAP;
	++rbf.size;
	
	if(rbf.size > RBF_CAP - 5) {
		enableCTS();
	}
	
	if(!dispatching) {
		dispatching = 1;
		while(rbf.size != 0)
		{
			uint8_t byte = rbf.buffer[rbf.tailIdx];
			rbf.tailIdx = (rbf.tailIdx + 1) % RBF_CAP;
			--rbf.size;
			if(rbf.size < 16) {
				disableCTS();
			}
			sei();
			_rcvCallback(byte);
			cli();
		}
		dispatching = 0;
	}
}

ISR(USART3_UDRE_vect, ISR_BLOCK)
{
	disableEmptyInterrupt();
	if(sending == 1 && pauseSending == 0) {
		sending = 0;
		UDR3 = leftover;
		enableEmptyInterrupt();
	}
	else if(sending == 0 && pauseSending == 0) {
		sei();
		_sndCallback();
	}
}

ISR(PCINT1_vect, ISR_BLOCK) {
	if (getRTS() == 0 && pauseSending == 1) {
		pauseSending = 0;
		halWT41FcUartSend(leftover);
	}
}

error_t halWT41FcUartSend(uint8_t byte)
{
	leftover = byte;
	if(getRTS() == 1) {
		pauseSending = 1;
	}
	sending = 1;
	enableEmptyInterrupt();
	return SUCCESS;
}

static void setupTimer(void)
{
	TCCR3B |= PRESCALAR | (1 << WGM32);
	TCNT3 = 0;
	OCR3A = TICKS;
	
	TIMSK3 |= (1 << OCIE3A);
}

error_t halWT41FcUartInit(void (*sndCallback)(), void (*rcvCallback)(uint8_t))
{	
	// set callbacks
	_sndCallback = sndCallback;
	_rcvCallback = rcvCallback;
		
	// init ring buffer
	rbf_init();
		
	// set baudrate
	UBRR3 = UBRR_VAL;
		
	// set control registers
	clear_port_bits(UCSR3C, 0xFF);
	set_port_bits(UCSR3C, (1 << UCSZ31) | (1 << UCSZ30));
	
	setAsOut(&PORTJ, &DDRJ, (1 << PJ5) | (1 << CTS));
	setAsInNoPullUp(&PORTJ, &DDRJ, (1 << RTS));	
	
	// set pc interrupt
	PCICR |= (1 << PCIE1);
	PCMSK1 |= (1 << PCINT11);
	
	disableCTS();
	disableEmptyInterrupt();
	
	// pull RST low
	set_port_bits(PORTJ, (1 << PJ5));
	clear_port_bits(PORTJ, (1 << PJ5));
	setupTimer();
	
	return SUCCESS;
}

static void enableEmptyInterrupt(void)
{
	set_port_bits(UCSR3B, (1 << UDRIE3));
}

static void disableEmptyInterrupt(void)
{
	clear_port_bits(UCSR3B, (1 << UDRIE3));
}

static void enableCTS(void)
{
	set_port_bits(PORTJ, (1 << CTS));
}

static void disableCTS(void)
{
	clear_port_bits(PORTJ, (1 << CTS));
}

static uint8_t getRTS(void)
{
	return (PINJ & (1 << RTS));
}

