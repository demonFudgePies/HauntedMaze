#include <avr/io.h>
#include "utils.h"

void
setAsOut(volatile uint8_t *port, volatile uint8_t *ddr, uint8_t mask)
{
	clear_port_bits(*port, mask);
	set_port_bits(*ddr, mask); 
}

void 
setAsIn(volatile uint8_t *port, volatile uint8_t *ddr, uint8_t mask)
{
	set_port_bits(*port, mask);
	clear_port_bits(*ddr, mask);
}

void
setAsInNoPullUp(volatile uint8_t *port, volatile uint8_t *ddr, uint8_t mask)
{
	clear_port_bits(*port, mask);
	clear_port_bits(*ddr, mask); 
}
