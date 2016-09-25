#include "sdcard/spi.h"
#include "sdcard/sdcard.h"
#include "mp3/mp3.h"

// Start address of Poker Face (C)
#define GAGA_START_ADDR 16429984
// Length of Poker Face (C)
#define GAGA_LEN 860256
// How many blocks should be sent in one go.
#define BLOCKS_PER_TASK 32

// Buffer to store sd card data into
static sdcard_block_t sdBuffer;
// The current next address to read music from.
static uint32_t currMusicAddr = GAGA_START_ADDR;
// Can the mp3 handle more data?
static volatile uint8_t sendMoreGaga = 0;
// The next value the volume should be set to.
static volatile uint8_t newVolume = 0;

/**
 * @brief A callback function given to the mp3 module to be called when it can handle more data.
 * Sets an internal variable to allow sending more data.
 */
static void gagaCallback(void);

void musicInit(void)
{
	spiInit();
	sdcardInit();
	mp3Init(gagaCallback);
}

void gagaCallback(void)
{
	sendMoreGaga = 1;
}

void volumeCb(uint8_t measure)
{
	// interpreting measure as 16 bit fixed point of format 8.8
	uint16_t linearVolume = 255 - measure;
	linearVolume *= linearVolume;
	linearVolume >>= 8;
	linearVolume *= linearVolume;
	linearVolume >>= 8;
	newVolume = 255 - linearVolume;
}

void musicBckg(void)
{
	if(newVolume != 0) {
		mp3SetVolume(newVolume);
		newVolume = 0;
	}
	uint8_t sentBlocks = 0;
	while(sentBlocks < BLOCKS_PER_TASK && sendMoreGaga) {
		sdcardReadBlock(currMusicAddr, sdBuffer);
		currMusicAddr += BLOCK_SIZE;
		if(currMusicAddr >= GAGA_START_ADDR + GAGA_LEN){
			currMusicAddr = GAGA_START_ADDR;
		}
		mp3SendMusic(sdBuffer);
		++sentBlocks;
		if(mp3Busy()) {
			sendMoreGaga = 0;
		}
	}
}
