#include "glcd.h"
#include "../utils/utils.h"
#include "hal_glcd.h"

#include <avr/pgmspace.h> 
#include <stdlib.h>

/**
 * @brief When this define exists a framebuffer is used.
 * 
 * If the define is deleted, the code defaults back to direct writing.
 * Slow, and very much not recomended.
 */
#define USE_FRAME_BUFFER

#ifdef USE_FRAME_BUFFER
#define FRAME_BUFFER_HEIGHT (SCREEN_HEIGHT / 8)
static uint8_t frameBuffer[SCREEN_WIDTH][FRAME_BUFFER_HEIGHT];
#endif

static uint8_t yShift;

void glcdFlushFramebuffer(void)
{
#ifdef USE_FRAME_BUFFER
	for(uint8_t j = 0; j < FRAME_BUFFER_HEIGHT; ++j) {
		halGlcdSetAddress(0, j);
		for(uint8_t i = 0; i < SCREEN_WIDTH; ++i) {
			halGlcdWriteData(frameBuffer[i][j]);
		}
	}
#endif
}

void glcdInit(void)
{
	halGlcdInit();
	glcdFillScreen(0x00);
}

/*
 * @param x coord in range 0 <= x <= 127
 * @param y coord in range 0 <= y <= 63
 */
void glcdSetPixel(const uint8_t x, const uint8_t y)
{
	uint8_t yRoundDown = y / 8;
	uint8_t yInByte = y % 8;
	
#ifdef USE_FRAME_BUFFER
	frameBuffer[x][yRoundDown] |= (1 << yInByte);
#else
	halGlcdSetAddress(x, yRoundDown);
	uint8_t currVal = halGlcdReadData();
	currVal |= (0x01 << yInByte);
	halGlcdSetAddress(x, yRoundDown);
	halGlcdWriteData(currVal);
#endif

}

void glcdClearPixel(const uint8_t x, const uint8_t y)
{
	uint8_t yRoundDown = y / 8;
	uint8_t yInByte = y % 8;
	
#ifdef USE_FRAME_BUFFER
	frameBuffer[x][yRoundDown] &= ~(1 << yInByte);
#else
	halGlcdSetAddress(x, yRoundDown);
	uint8_t currVal = halGlcdReadData();
	currVal &= ~(0x01 << yInByte);
	halGlcdSetAddress(x, yRoundDown);
	halGlcdWriteData(currVal);
#endif
}

void glcdInvertPixel(const uint8_t x, const uint8_t y)
{
	uint8_t yRoundDown = y / 8;
	uint8_t yInByte = y % 8;
#ifdef USE_FRAME_BUFFER
	frameBuffer[x][yRoundDown] ^= (1 << yInByte);
#else
	halGlcdSetAddress(x, yRoundDown);
	uint8_t currVal = halGlcdReadData();
	currVal ^= (0x01 << yInByte);
	halGlcdSetAddress(x, yRoundDown);
	halGlcdWriteData(currVal);
#endif
}

void glcdDrawLine(const xy_point p1, const xy_point p2,
					void (*drawPx)(const uint8_t, const uint8_t))
{
	int8_t w = p2.x - p1.x;
    int8_t h = p2.y - p1.y;
    int8_t dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0 ;
    if(w < 0) {
    	dx1 = dx2 = -1;
    }
    else if(w > 0) {
    	dx1 = dx2 = 1;    	
    }
    if(h < 0) {
    	dy1 = -1;
    } 
    else if(h > 0) { 
    	dy1 = 1;
    }
    int8_t longest = ABS(w);
    int8_t shortest = ABS(h);
    if (longest <= shortest) {
        longest = ABS(h);
        shortest = ABS(w);
        if(h<0) {
        	dy2 = -1;
        } 
        else if(h>0) {
        	dy2 = 1;
        }
        dx2 = 0;            
    }
    int16_t error = longest / 2;
    int8_t x = p1.x, y = p1.y;    
    int8_t i;
    for (i=0;i<=longest;i++) {
        drawPx(x,y);
        error += shortest;
        if (error >= longest) {
            error -= longest;
            x += dx1;
            y += dy1;
        } else {
            x += dx2;
            y += dy2;
        }
    }
}

void glcdDrawVertical(const uint8_t x,
					void (*drawPx)(const uint8_t, const uint8_t))
{
	uint8_t y;
	for(y = 0; y < 64; ++y) {
		drawPx(x, y);
	} 
}

void glcdDrawRect(const xy_point p1, const xy_point p2,
				void (*drawPx)(const uint8_t, const uint8_t))
{
	xy_point topLeft, topRight, bottomLeft, bottomRight;
	topLeft = p1;
	topRight.x = p2.x;
	topRight.y = p1.y;
	bottomLeft.x = p1.x;
	bottomLeft.y = p2.y;
	bottomRight = p2;
	
	--topRight.x;
	glcdDrawLine(topLeft, topRight, drawPx);
	++topRight.x;
	
	--bottomRight.y;
	glcdDrawLine(topRight, bottomRight, drawPx);
	++bottomRight.y;
	
	++bottomLeft.x;
	glcdDrawLine(bottomRight, bottomLeft, drawPx);
	--bottomLeft.x;
	
	++topLeft.y;
	glcdDrawLine(bottomLeft, topLeft, drawPx);
	--topLeft.y;
}

void glcdFillScreen(const uint8_t pattern)
{
	
#ifdef USE_FRAME_BUFFER
	for(uint8_t j = 0; j < FRAME_BUFFER_HEIGHT; ++j) {
		for(uint8_t i = 0; i < SCREEN_WIDTH; ++i) {
			frameBuffer[i][j] = pattern;
		}
	}
#else
	uint8_t x, y;
	halGlcdSetAddress(0, 0);
	for(y = 0; y < 8; ++y) {
		halGlcdSetAddress(0, y);
		for(x = 0; x < 128; ++x) {
			halGlcdWriteData(pattern);
		}
	}
#endif

}

void glcdDrawArrayPgm(PGM_P array, uint16_t len)
{
#ifdef USE_FRAME_BUFFER
	uint16_t arrIdx = 0;
	for(uint8_t j = 0; j < FRAME_BUFFER_HEIGHT; ++j) {
		for(uint8_t i = 0; i < SCREEN_WIDTH; ++i) {
			frameBuffer[i][j] = pgm_read_byte(&array[arrIdx++]);
		}
	}
#else
	uint8_t arrIdx;
	halGlcdSetAddress(0, 0);
	for(uint8_t y = 0; y < 8; ++y) {
		halGlcdSetAddress(0, y);
		for(uint8_t x = 0; x < 128; ++x) {
			halGlcdWriteData(pgm_read_byte(&array[arrIdx++]));
		}
	}
#endif
}

void glcdSetYShift(uint8_t yshift)
{
	yShift = yshift;
	halGlcdSetYShift(yshift);
}

uint8_t glcdGetYShift(void)
{
	return yShift;
}

void glcdDrawHorizontal(const uint8_t y,
					void (*drawPx)(const uint8_t, const uint8_t))
{
	uint8_t x;
	for(x = 0; x < 128; ++x) {
		drawPx(x, y);
	} 
}

void glcdFillRect(const xy_point p1, const xy_point p2,
				void (*drawPx)(const uint8_t, const uint8_t))
{
	uint8_t x, y;
	for(y = p1.y; y <= p2.y; ++y)
	{
		for(x = p1.x; x <= p2.x; ++x)
		{
			drawPx(x, y);
		}
	}
}

void glcdDrawChar(const char c, const xy_point p, const font* f,
				void (*drawPx)(const uint8_t, const uint8_t))
{
	int16_t chOffset = (c - f->startChar); // c starts at the chOffset-th byte in our array
	chOffset *=  f->width;
	
	uint8_t currByte, currBit;
	for(currByte = 0; currByte < f->width; ++currByte) {
		uint8_t chByte =  pgm_read_byte(&(f->font[chOffset + currByte]));
		for(currBit = 0; currBit < 8; ++currBit) {
			if((chByte & 0x01) != 0) {
				drawPx(p.x + currByte, p.y + currBit);
			}
			chByte >>= 1;
		}
	}
}

void glcdDrawText(const char *text, const xy_point p, const font* f,
				void (*drawPx)(const uint8_t, const uint8_t))
{
	uint8_t txtIdx;
	xy_point currPos = p;
	for(txtIdx = 0; text[txtIdx] != '\0'; ++txtIdx) {
		switch(text[txtIdx])
		{
		case '\n':
			currPos.y += f->lineSpacing;
			currPos.x = p.x;
			break;
		default:
			glcdDrawChar(text[txtIdx], currPos, f, drawPx);
			currPos.x += f->charSpacing;
			break;
		}
	}
}

void  glcdDrawTextPgm(PGM_P text, const xy_point p, const font* f,
					void (*drawPx)(const uint8_t, const uint8_t))
{
	uint8_t txtIdx;
	xy_point currPos = p;
	uint8_t currChar;
	for(txtIdx = 0; ( currChar = pgm_read_byte(&(text[txtIdx])) ) != '\0'; ++txtIdx) {
		switch(currChar)
		{
		case '\n':
			currPos.y += f->lineSpacing;
			currPos.x = p.x;
			break;
		default:
			glcdDrawChar(currChar, currPos, f, drawPx);
			currPos.x += f->charSpacing;
			break;
		}
	}
}
