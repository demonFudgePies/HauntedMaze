#include <avr/io.h>

#include "font/font.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// 2D 8bit unsigned cartesian points
typedef struct xy_point_t {
	uint8_t x, y;
} xy_point;

/**
 * @brief Flushes entire framebuffer to the screen 
 * if the framebuffer flag was set during compilation.
 */
void glcdFlushFramebuffer(void);

void glcdInit(void);

void glcdSetPixel(const uint8_t x, const uint8_t y);

void glcdClearPixel(const uint8_t x, const uint8_t y);

void glcdInvertPixel(const uint8_t x, const uint8_t y);

void glcdDrawLine(const xy_point p1, const xy_point p2,
				void (*drawPx)(const uint8_t, const uint8_t));

void glcdDrawRect(const xy_point p1, const xy_point p2,
				void (*drawPx)(const uint8_t, const uint8_t));

void glcdFillScreen(const uint8_t pattern);

void glcdSetYShift(uint8_t yshift);

uint8_t glcdGetYShift(void);

void glcdDrawVertical(const uint8_t x,
					void (*drawPx)(const uint8_t, const uint8_t));

void glcdDrawHorizontal(const uint8_t y,
					void (*drawPx)(const uint8_t, const uint8_t));

void glcdFillRect(const xy_point p1, const xy_point p2,
				void (*drawPx)(const uint8_t, const uint8_t));
				
void glcdDrawChar(const char c, const xy_point p, const font* f,
				void (*drawPx)(const uint8_t, const uint8_t));
				
void glcdDrawText(const char *text, const xy_point p, const font* f,
				void (*drawPx)(const uint8_t, const uint8_t));
				
void glcdDrawTextPgm(PGM_P text, const xy_point p, const font* f,
					void (*drawPx)(const uint8_t, const uint8_t));

/**
 * @brief Flushes an entire array size of 1024 * 8 to the screen.
 *
 * Writes to the framebuffer if enabled. If not, writes directly.
 */			
void glcdDrawArrayPgm(PGM_P array, uint16_t len); 					
