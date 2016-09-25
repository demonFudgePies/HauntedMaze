#include "renderer.h"
#include "glcd/glcd.h"
#include <avr/pgmspace.h>
#include "glcd/font/Standard5x7.h"
#include <stdlib.h>

#define START_SCREEN_LEN 1024
#define NEGATIVE_TICKS 16
#define PAC_TICKS 4

// top left corner of the camera in world space 
static int16_t camX = 0;
static int16_t camY = 0;

// an array containing the information on how pacman looks. Not in PROGMEM because they are edited.
static uint8_t pacman[4][4] = { {0, 1, 1, 0}, {1, 1, 1, 1}, {1, 1, 0, 0}, {0, 1, 1, 1} };

// pointers to the player position
static int16_t* _playerX;
static int16_t* _playerY;

// the size of the player
static uint8_t playerSize;

// how many times the update animation routine has been called
static uint16_t ticksPassed = 0;

/**
 * @brief Draws one tile of the maze and a point in it, if point is set
 * @param tile Maze tile to be drawn
 * @param point Draw the point in tile if point is 1. 
 * @param p1X Top left x tile coordinate in world space.
 * @param p1Y Top left y tile coordinate in world space.
 * @param p2X Bottom right x tile coordinate in world space.
 * @param p2Y Bottom right y tile coordinate in world space.
 */
static void drawTile(maze_tile tile, uint8_t point, int16_t p1X, int16_t p1Y,
					int16_t p2X, int16_t p2Y, void (*drawPx)(const uint8_t, const uint8_t));
					
// image for the start screen			
extern const uint8_t startScreen[START_SCREEN_LEN] PROGMEM;
extern const char winMessage[] PROGMEM;
extern const char OK[] PROGMEM;
extern const char newGameMessage[] PROGMEM;
extern const char connMessage[] PROGMEM;
extern const char loseMessage[] PROGMEM;
extern const char scoreMessage[] PROGMEM;
extern const char pressMessage[] PROGMEM;

// should the negative of a button be drawn
static uint8_t okNegative = 0;

void renderGhosts(ghost ghosts[], uint8_t ghostCount)
{
	uint16_t gX, gY;
	for(uint8_t k = 0; k < ghostCount; ++k) {
		ghost currGhost = ghosts[k];
		for(uint8_t i = 0; i < GHOST_W; ++i) {
			for(uint8_t j = 0; j < GHOST_H; ++j) {
				gX = currGhost.x + i - camX;
				gY = currGhost.y + j - camY;
				if(gX >= 0 && gY >= 0 && 
					gX < SCREEN_WIDTH && gY < SCREEN_HEIGHT)
				{
					if(!(i == 1 && j == 2) && 
						!(i == 3 && j == 2) &&
						!(i == 0 && j == 0) &&
						!(i == GHOST_W - 1 && j == 0)){ // hardcoded eyes and legs
						glcdSetPixel(gX, gY);
					}
					else {
						glcdClearPixel(gX, gY);
					}
				}
			}
		} 
	}
}

uint8_t updateStartScreen(void)
{
	static uint8_t currRow = 0;
	for(uint8_t i = 0; i < SCREEN_WIDTH; ++i) {
		glcdInvertPixel(i, currRow);
	}
	glcdFlushFramebuffer();
	++currRow;
	if(currRow == SCREEN_HEIGHT) {
		return 1;
	}
	return 0;
}

void drawStartScreen(void)
{
	glcdDrawArrayPgm((PGM_P)startScreen, START_SCREEN_LEN * 8);
	glcdFlushFramebuffer();
}

void drawEndScreen(uint8_t won, uint16_t score)
{
	char buffer[8];
	itoa(score, buffer, 10);
	xy_point scoreLoc = {32, SCREEN_HEIGHT - Standard5x7.lineSpacing};
	glcdDrawTextPgm((PGM_P)scoreMessage, scoreLoc, &Standard5x7, glcdSetPixel);
	scoreLoc.x += strlen_P(scoreMessage) * Standard5x7.charSpacing  + 10;
	glcdDrawText(buffer, scoreLoc, &Standard5x7, glcdSetPixel);
		
	xy_point textLoc = {4, 10};
	PGM_P mssg;
	if(won == 1) {
		mssg = (PGM_P)winMessage;
	}
	else if(won == 0) {
		mssg = (PGM_P)loseMessage;
	}
	
	textLoc.x = SCREEN_WIDTH / 2 - Standard5x7.charSpacing * (strlen_P(mssg) / 2);
	glcdDrawTextPgm((PGM_P)mssg, textLoc, &Standard5x7, glcdInvertPixel);
	
	textLoc.y += Standard5x7.lineSpacing;
	textLoc.x = SCREEN_WIDTH / 2 - Standard5x7.charSpacing * (strlen_P(newGameMessage) / 2) - Standard5x7.charSpacing / 2;
	glcdDrawTextPgm((PGM_P)newGameMessage, textLoc, &Standard5x7, glcdInvertPixel);
	
	textLoc.y += Standard5x7.lineSpacing * 2;
	textLoc.x = SCREEN_WIDTH / 2 - Standard5x7.charSpacing * (strlen_P(OK) / 2);
	
	xy_point esRectStart; 
	xy_point esRectEnd;
	esRectStart = textLoc;
	esRectStart.x -= 10;
	esRectStart.y -= 3;
	esRectEnd.y = esRectStart.y + Standard5x7.lineSpacing;
	esRectEnd.x = SCREEN_WIDTH / 2 + Standard5x7.charSpacing * (strlen_P(OK) / 2);
	esRectEnd.x += 8;
	esRectEnd.y += 4;
	
	if(!okNegative) {
		glcdDrawTextPgm((PGM_P)OK, textLoc, &Standard5x7, glcdSetPixel);
	} else {
		glcdFillRect(esRectStart, esRectEnd, glcdSetPixel);
		glcdDrawTextPgm((PGM_P)OK, textLoc, &Standard5x7, glcdClearPixel);
	}
}

void drawConnectingScreen(void)
{
	xy_point textLoc, rectStart, rectEnd;
	textLoc.x = SCREEN_WIDTH / 2 - Standard5x7.charSpacing * (strlen_P(connMessage) / 2) + 1;
	textLoc.y = 20;
	rectStart = rectEnd = textLoc;
	rectStart.x -= 5;
	rectStart.y -= 4;
	rectEnd.x += Standard5x7.charSpacing * strlen_P(connMessage) + 3;
	rectEnd.y += 11;
	
	if(!okNegative) {
		glcdDrawTextPgm((PGM_P)connMessage, textLoc, &Standard5x7, glcdSetPixel);
	} else {
		glcdFillRect(rectStart, rectEnd, glcdSetPixel);
		glcdDrawTextPgm((PGM_P)connMessage, textLoc, &Standard5x7, glcdClearPixel);
	}
	
	xy_point pressLoc = {SCREEN_WIDTH / 2- Standard5x7.charSpacing * (strlen_P(pressMessage) / 2), SCREEN_HEIGHT - Standard5x7.lineSpacing * 2};
	glcdDrawTextPgm((PGM_P)pressMessage, pressLoc, &Standard5x7, glcdSetPixel);
}

void updateAnimations(void)
{
	++ticksPassed;
	if(ticksPassed % NEGATIVE_TICKS == 0) {
		okNegative = !okNegative;
	}
	if(ticksPassed % PAC_TICKS == 0) {
		pacman[2][3] = !pacman[2][3];
		pacman[2][2] = !pacman[2][2];
	}
}

void updateCamera(void)
{
	int16_t playerX = *_playerX;
	int16_t playerY = *_playerY;
	
	if(playerX >= SCREEN_WIDTH / 2) {
		camX = playerX - SCREEN_WIDTH / 2;	
		if(camX + SCREEN_WIDTH > MAZE_WIDTH * TILE_SIZE) {
			camX = MAZE_WIDTH * TILE_SIZE - SCREEN_WIDTH;
		}
	}
	else {
		camX = 0;
	}
	
	if(playerY >= SCREEN_HEIGHT / 2) {
		camY = playerY - SCREEN_HEIGHT / 2;	
		if(camY + SCREEN_HEIGHT > MAZE_HEIGHT * TILE_SIZE) {
			camY = MAZE_HEIGHT * TILE_SIZE - SCREEN_HEIGHT;
		}
	}
	else {
		camY = 0;
	}
}

void endRender(void)
{	
	glcdFlushFramebuffer();
}

void startRender(void)
{
	glcdFillScreen(0x00);
}

void rendererInit(int16_t* playerX, int16_t* playerY, uint8_t psize)
{
	glcdInit();
	_playerX = playerX;
	_playerY = playerY;
	playerSize = psize;
}

void renderPlayer(void)
{
	int16_t playerX = *_playerX;
	int16_t playerY = *_playerY;
	
	for(uint8_t i = 0; i < playerSize; ++i) {
		for(uint8_t j = 0; j < playerSize; ++j) {
			if(pacman[j][i]) {
				glcdSetPixel(playerX + i - camX, playerY + j - camY);
			}
		}
	} 
}

void renderMaze(maze_tile maze[][MAZE_HEIGHT], uint8_t points[][MAZE_HEIGHT])
{
	uint16_t tileStartX = (camX / TILE_SIZE);
	uint16_t xOffset = camX % TILE_SIZE;
	uint16_t tileStartY = (camY / TILE_SIZE);
	uint16_t yOffset = camY % TILE_SIZE;
	int16_t p1X, p1Y, p2X, p2Y;
	for(uint8_t i = 0; i < SCREEN_WIDTH + xOffset; i += TILE_SIZE) {
		for(uint8_t j = 0; j < SCREEN_HEIGHT + yOffset; j += TILE_SIZE) {
			p1X = i - xOffset;
			p1Y = j - yOffset;
			p2X = i + TILE_SIZE - 1 - xOffset;
			p2Y = j + TILE_SIZE - 1 - yOffset;
			drawTile(maze[i / TILE_SIZE + tileStartX][j / TILE_SIZE + tileStartY], points[i / TILE_SIZE + tileStartX][j / TILE_SIZE + tileStartY], p1X, p1Y, p2X, p2Y, glcdSetPixel);
		}
	}
}


// points in screen space... probably... what the hell did I do here?!
static void drawTile(maze_tile tile, uint8_t point, int16_t p1X, int16_t p1Y,
					int16_t p2X, int16_t p2Y, void (*drawPx)(const uint8_t, const uint8_t))
{
	// clamp values
	xy_point topLeft = { 
						((p1X >= 0)?p1X:0), 
						((p1Y >= 0)?p1Y:0) 
						 }; //p1
	xy_point topRight = { 
						((p2X < SCREEN_WIDTH)?p2X:SCREEN_WIDTH - 1), 
						((p1Y >= 0)?p1Y:0) 
						}; //p2X, p1Y};
	xy_point bottomLeft = { 
						((p1X >= 0)?p1X:0), 
						((p2Y < SCREEN_HEIGHT)?p2Y:SCREEN_HEIGHT - 1) 
						}; //p1X, p2Y}; 
	xy_point bottomRight = {
						((p2X < SCREEN_WIDTH)?p2X:SCREEN_WIDTH - 1), 
						((p2Y < SCREEN_HEIGHT)?p2Y:SCREEN_HEIGHT - 1) 
						}; // p2;
	
	if(point != 0) {
		int16_t pointTLX = p1X + 3;
		int16_t pointTLY = p1Y + 3;
		
		for(int16_t i = 0; i < 2; ++i) {
			for(int16_t j = 0; j < 2; ++j) {
				if(pointTLX + i >= 0 && pointTLX + i < SCREEN_WIDTH &&
					pointTLY + j >= 0 && pointTLY + j < SCREEN_HEIGHT) {
					glcdSetPixel(pointTLX + i, pointTLY + j);
				}
			}
		}
	}
	 
	// above
	if(!tile.tile.freeTop && p1Y >= 0) {
		glcdDrawLine(topLeft, topRight, drawPx);
	}
	
	// left
	if(!tile.tile.freeLeft && p1X >= 0) {
		glcdDrawLine(bottomLeft, topLeft, drawPx); 
	}
	
	// right
	if(!tile.tile.freeRight && p2X < SCREEN_WIDTH) {
		glcdDrawLine(topRight, bottomRight, drawPx); 
	}
	
	// bottom
	if(!tile.tile.freeBottom && p2Y < SCREEN_HEIGHT) {
		glcdDrawLine(bottomRight, bottomLeft, drawPx); 
	}
	
	if(tile.tile.isEnd) {
		glcdDrawLine(bottomRight, topLeft, drawPx);
		glcdDrawLine(topRight, bottomLeft, drawPx);
	}
}
