#include <avr/interrupt.h>
#include "adc.h"

#define PRESC ((1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0)) // prescalar 128 Khz

#define SRA (PRESC | (1 << ADEN) | (1 << ADIE) | (1 << ADSC)); // not (!) free running mode, enable, enable interrupt, start measuring

#define DIFF_MUX ((1 << MUX0) | (1 << MUX1) | (1 << MUX2) | (1 << MUX3) | (1 << REFS0) | (1 << REFS1))//001111 -> differential mode between AC2 and AC3
#define ADC0_MUX ((1 << ADLAR) | (1 << REFS0)) // reference voltage, read only top 8 bits

#define TIMER_PRESCALAR (1 << CS41 | 1 << CS40)
#define TICKS 470

// types of channels
typedef enum {
	ADC0_ACTIVE,
	DIFF_ACTIVE
} ADC_CHANNEL;

// whether to ignore the next read (needed when switching to differential)
static volatile uint8_t ignoreNext = 0;
// currently active channel
static volatile ADC_CHANNEL currChannel = ADC0_ACTIVE;

// callback functions
static void (*_volumeCallback)(uint8_t measure);
static void (*_randCallback)(uint8_t measure);

// Timer ISR which activates the next measuring.
ISR (TIMER4_COMPA_vect)
{
	ADCSRA |= (1 << ADSC);
}

// ADC ISR for when the measuring is over.
ISR(ADC_vect, ISR_BLOCK) 
{
	uint16_t measure = ADC;
	if (currChannel == ADC0_ACTIVE) {
		_volumeCallback((uint8_t)(measure >> 8));
		currChannel = DIFF_ACTIVE;
		PORTC = (measure >> 8);
		ADMUX = DIFF_MUX;
		ignoreNext = 1;
	}
	else if (currChannel == DIFF_ACTIVE) {
		if(ignoreNext != 0) {
			ignoreNext = 0;
		}
		else {
			_randCallback((uint8_t)measure);
			currChannel = ADC0_ACTIVE;
			DDRC = 0xFF;
			ADMUX = ADC0_MUX;
		}
	};
}

void adcInit(void (*volumeCallback)(uint8_t measure),
				void (*randCallback)(uint8_t measure))
{	
	ADMUX = ADC0_MUX;
	ADCSRA = SRA;
	_volumeCallback = volumeCallback;
	_randCallback = randCallback;
	
	TCCR4B |= TIMER_PRESCALAR | (1 << WGM42);
	TCNT4 = 0;
	OCR4A = TICKS;
	
	TIMSK4 |= (1 << OCIE4A);
	//DDRL = 0xFF;
	ADCSRA |= (1 << ADSC);
}
