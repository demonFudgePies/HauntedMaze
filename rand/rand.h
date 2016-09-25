
#ifndef RAND_H
#define RAND_H

#include <stdint.h>

uint8_t rand_shift(uint8_t in);

void rand_feed(uint8_t in);

uint8_t rand1(void);

uint16_t rand16(void);

#endif

