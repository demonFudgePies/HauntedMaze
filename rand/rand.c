#include "rand.h"
#include <util/atomic.h>

static uint16_t lfsr = 1;
//static uint16_t poly = 0x80E3;


uint8_t rand_shift(uint8_t in)
{
	uint8_t out = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		asm volatile(
			"lsr %2 \n" // shift in into the carry
			"ror %B1 \n" // shift high byte of lfsr, with carry
			"ror %A1 \n" // shift low byte of lfsr, with carry
			"brcc no_carry_%= \n" // skip if there was no carry last tune
			"ldi %2, %3\n"
			"eor %B1, %2\n" // xor high bit
			"ldi %2, %4\n"
			"eor %A1, %2\n" // xor low bit
			"no_carry_%=: \n" // label
			"rol %0 \n"// shift carry into out
			:
			"+r"(out),
			"+r"(lfsr),
			"+r"(in)
			:
			"M" (0x80),
			"M" (0xE3)
		);
	}
	return out;
}

void rand_feed(uint8_t in)
{
	rand_shift(in);
}


uint8_t rand1()
{
	return rand_shift(0);
}

uint16_t rand16()
{
	uint16_t rn = 0;
	for(int i = 0; i < 16; ++i) {
		rn <<= 1;
		rn |= rand_shift(0);
	}
	return rn;
}


