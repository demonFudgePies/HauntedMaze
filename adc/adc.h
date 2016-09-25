#include <avr/io.h>

/**
 * @brief Initializes the ADC module, and takes two callback functions.
 * @param volumeCallback Called when a new volume value is read.
 * @param randCallback Called when a new rng seed is generated.
 */ 
void adcInit(void (*volumeCallback)(uint8_t measure), 
				void (*randCallback)(uint8_t measure));
