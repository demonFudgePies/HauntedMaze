#include "spi.h"
#include "../utils/utils.h"

#define DDR_SPI DDRB
#define MISO DDB3
#define MOSI DDB2 
#define SCK DDB1
#define SS DDB0

void spiInit(void)
{
	set_port_bits(DDR_SPI, (1 << MOSI) | (1 << SCK));
	set_port_bits(SPCR, (1 << SPE) | (1 << MSTR));
	//clear_port_bits(SPCR, (1 << CPOL) | (1 << CPHA));
}

void spiSend(uint8_t data)
{
	SPDR = data; // send data
	while(!(SPSR & (1 << SPIF))); // wait for sent flag
}

uint8_t spiReceive(void)
{
	spiSend(0xFF); // send dummy data
	while(!(SPSR & (1 << SPIF))); // wait for flag
	uint8_t data = SPDR; // recieve data
	return data;
}

void spiSetPrescaler(spi_prescaler_t prescaler)
{
	clear_port_bits(SPCR, (1 << SPR1) | (1 << SPR0));
	set_port_bits(SPCR, prescaler);
}
