/**
 * @author Nikola Dodik
 * @date 8 May 2016
 * @brief The main file for the annoying labyrinth game.
 * 
 * On a personal note, I came to realize that the attribute
 * for the project title was aptly chosen.
 */

#include <avr/io.h>
#include <avr/sleep.h>
#include <stdlib.h>

#include "glcd/glcd.h"
#include "renderer.h"
#include "utils/utils.h"
#include "music_handler.h"
#include "adc/adc.h"
#include "rand/rand.h"
#include "mazeGen/mazeGen.h"
#include "wiimote/wii_user.h"
#include "game_logic.h"
#include "ghost.h"

#include <util/atomic.h>

#define TIMER_PRESCALAR (1 << CS52 | 1 << CS50)
#define TICKS 12499

/**
 * @brief How many labyrinth tiles to generate in
 * one iteration of loading
 */
#define MAZE_LOAD_INC 32

/** 
 * @brief How many tiles to leave free when placing the ghosts
 * in both the x and the y direcion
 */
#define GHOST_FREE_TILES 4
 
/**
 * @brief Set if there was a timer tick.
 *
 * Timer ticked is equal to 1 if there was a timer 
 * tick since the last time it was manually reset.
 */
static volatile uint8_t timerTicked = 0;

/**
 * @brief Should be set to 1 after the intro 
 * animation is finished playing
 *
 * Tells the timer to start going faster,
 * i.e. tick at the actual gameplay speed.
 */
static volatile uint8_t introOver = 0;

/**
 * @brief ISR for the main file timer.
 *
 * Sets timerTicked to 1. If introOver is
 * set to 1, it will set the speed at 20Hz,
 * and set introOver to 0.
 */
ISR (TIMER5_COMPA_vect)
{
	if(introOver == 1) {
		TCCR5B &= ~(1 << CS52 | 1 << CS50);
		TCCR5B |= (1 << CS51 | 1 << CS50);
		OCR5A &= 0x00;
		OCR5A |= 12499;
		introOver = 0;
	}
	timerTicked = 1;
}

/**
 * @brief Sets up the game timer.
 *
 * Sets the timer in CTCA mode at a slower
 * speed than for the game loop, since an intro
 * sequence is expected.
 */
static void setupTimer(void)
{
	TCCR5B |= TIMER_PRESCALAR | (1 << WGM52);
	TCNT5 = 0;
	OCR5A = TICKS;
	
	TIMSK5 |= (1 << OCIE5A);
}


/**
 * @brief Possible states during gameplay.
 */
typedef enum {
	START_STATE,
	LOADING_STATE,
	GAME_STATE,
	WIN_STATE,
	LOSE_STATE
} game_state_t;

/**
 * @brief Possible sub-states during the
 * LOADING_STATE game state.
 */
typedef enum {
	LOADING_STARTED,
	LOADING_COMPLETE
} load_state_t;

/**
 * @brief Running flag. Always equals 1.
 */
static volatile uint8_t running = 1;

/**
 * @brief Contains the information about the current
 * gameplay state.
 */
static volatile game_state_t gameState = START_STATE;

/**
 * @brief Contains the information about the current
 * loading sub-state.
 */
static load_state_t loadState = LOADING_COMPLETE;

/**
 * @brief Labyrinth data.
 */
static maze_tile maze[MAZE_WIDTH][MAZE_HEIGHT];

/**
 * @brief 2D array with the point data.
 * 
 * The 2D array is the same size as maze. If an element
 * equals to 1, there is a collectible point at its 
 * coordinates.
 */
static uint8_t points[MAZE_WIDTH][MAZE_HEIGHT];

/**
 * @brief Information about each ghost.
 */
static ghost ghosts[GHOST_COUNT];

/**
 * @brief Player x coordinate in world space.
 */
static int16_t playerX = 2;

/**
 * @brief Player y coordinate in world space.
 */
static int16_t playerY = 2;

/**
 * @brief Size of the player in world space.
 */
static int16_t playerSize = 4;

/**
 * @brief Current player score.
 *
 * Equals the amount of picked up points.
 * Resets every new game.
 */
static uint16_t score;

/**
 * @brief Flag for when there is data from
 * the accelerometer.
 *
 * When equal to 1, userX, userY and userZ contain
 * new infomation from the accelrometer.
 * Should be reset after the information is processed.
 */
static volatile uint8_t accelData = 0;

/**
 * @brief Biggest X read from the accelerometer.
 *
 * Should be reset after it is parsed.
 */
static volatile int16_t userX = 0;

/**
 * @brief Biggest Y read from the accelerometer.
 *
 * Should be reset after it is parsed.
 */
static volatile int16_t userY = 0;

/**
 * @brief Biggest Z read from the accelerometer.
 *
 * Should be reset after it is parsed.
 */
static volatile int16_t userZ = 0;

/**
 * @brief Mac address of the wiimote.
 *
 * The game will try to connect to the
 * Wiimote with this mac address.
 */
extern const uint8_t _mac[1][6];

/**
 * @brief Flag which is set when the user
 * is connected to the game.
 */
static uint8_t userConnected = 0;

/**
 * @brief One iteration of the main program loop.
 */
static void mainIteration(void);

/**
 * @brief One iteration of the loading game loop.
 */
static void loadIteration(void);

/**
 * @brief One iteration of the game loop.
 */
static void gameIteration(void);

/**
 * @brief Parses the accelerometer data stored in userX
 * and userY, and places the player coordinate increments
 * into xInc and yInc.
 *
 * Predetermined mapping of input values to position incerments. Mappings
 * are based on what felt right to the developer.
 *
 * @param xInc memory at which to save the increment for x-coordinate
 * @param yInc memory at which to save the increment for y-coordinate
 */
static void parseAccelData(int8_t* xInc, int8_t* yInc);

/**
 * @brief Updates the player and ghost positions, checks for collisions,
 * updates game state accordingly. 
 * 
 * Prepares updates the game states for one frame. Should generally be used 
 * together with parseAccelData.
 * @param xInc Determines how much to increment the player on the x-axis.
 * @param yInc Determines how much to increment the player on the y-axis.
 */
static void updateScene(int8_t xInc, int8_t yInc);

/**
 * @brief Resets all variables so that a new level can be loaded.
 */
static void resetGameState(void);

/**
 * @brief Randomly places ghosts on the map.
 *
 * Ghosts are placed randomly on the map. First GHOST_FREE_TILES are left free both
 * in x and in y.
 * Makes sure that no ghost's direction is towards a wall.
 */
static void generateGhosts(void);

/**
 * @brief Callback function for button events on the Wiimote.
 *
 * If the game is in either WIN_STATE or LOSE_STATE states, and "A" is pressed,
 * it goes back into the LOADING_STATE state.
 * @param wii Wiimote id
 * @param buttonStates two bytes containing button state information.
 */
static void rcvButton(uint8_t wii, uint16_t buttonStates);

/**
 * @brief Callback function for accelerometer events on the Wiimote.
 *
 * Sets the accelData flag, and x, y or z are stored in userX, userY and userZ
 * respectively, if they are bigger then them.
 * @param wii Wiimote id
 * @param x Accelerometer x reading.
 * @param y Accelerometer y reading.
 * @param z Accelerometer z reading.
 */
static void rcvAccel(uint8_t wii, uint16_t x, uint16_t y, uint16_t z);

/**
 * @brief Callback function for the wiimote connection state change.
 *
 * If connection was unseccessful or if the user disconnected, tries to
 * reconnect. Is responsible for setting the userConnected flag (when 
 * the user is connected).
 * @param wii Wiimote id
 * @param status Whether the user successfully connected
 */
static void conCallback(uint8_t wii, connection_status_t status);

/**
 * @brief Dummy callback function for Wiimote leds. 
 *
 * Empty callback function.
 * @param wii Wiimote id
 * @param status Operation status.
 */
static void setLedsCallback(uint8_t wii, error_t status);

/**
 * @brief Callback function for turning on the accelerometer readings. 
 *
 * If status != SUCCESS, tries to connect again.
 * @param wii Wiimote id
 * @param status Operation status.
 */
static void setAccelCallback(uint8_t wii, error_t status);

/**
 * @brief Callback function given to the ADC module for feeding 
 * the random number generator
 * @param measure Value received from the ADC module.
 */
static void randCb(uint8_t measure);


/** 
 * @brief Program entry point.
 */
int main(void)
{
	rendererInit(&playerX, &playerY, playerSize);
	drawStartScreen();
	
	wiiUserInit(rcvButton, rcvAccel);
	musicInit();
	adcInit(volumeCb, randCb);
	sei();
	
	setupTimer();
	
	while(updateStartScreen() != 1) {
		sleep_cpu();
	}
	introOver = 1;
	
	if(!userConnected) {
		if(timerTicked != 0) {
			timerTicked = 0;
			wiiUserConnect(0, _mac, conCallback);
		}
	}
	
	//wiiUserSetRumbler(0, 1, setRumblerCallback);
	gameState = LOADING_STATE;
	while(running) {
		if(timerTicked != 0) {
			timerTicked = 0;
			mainIteration();
		}
		sleep_cpu();
	}
}

static void mainIteration(void)
{
	musicBckg();
	if(gameState == LOADING_STATE) {
		loadIteration();
	}
	else if(!userConnected) {
		updateAnimations();
		startRender();
		drawConnectingScreen();
		endRender();
	}
	else if(userConnected && gameState == GAME_STATE) {
		gameIteration();
		updateCamera();
		startRender();
		renderMaze(maze, points);
		renderPlayer();
		renderGhosts(ghosts, GHOST_COUNT);
		endRender();
	}
	else if(userConnected && (gameState == WIN_STATE || gameState == LOSE_STATE)) {
		updateAnimations();
		startRender();
		drawEndScreen(((gameState == WIN_STATE)?1:0), score);
		endRender();
	}
}

static void loadIteration(void)
{	
	static uint16_t remaining;
	static uint16_t start;
	
	if(loadState == LOADING_COMPLETE) {	
		remaining = MAZE_WIDTH * MAZE_HEIGHT;
		start = 0;
		resetGameState();
		loadState = LOADING_STARTED;
	}
	if(remaining > 0) {
		uint16_t smaller = (remaining < MAZE_LOAD_INC)?remaining:MAZE_LOAD_INC;
		generateMaze(maze, start, smaller);
		start += smaller;
		remaining -= smaller;
	}
	else {
		generateGhosts();
		loadState = LOADING_COMPLETE;
		gameState = GAME_STATE;
	}
}

static void gameIteration(void)
{	
	int8_t xInc = 0, yInc = 0;
	if(accelData) {
		parseAccelData(&xInc, &yInc);
		accelData = 0;
		userX = userY = userZ = 0;
	}
	updateScene(xInc, yInc);
	updateAnimations();
}

static void updateScene(int8_t xInc, int8_t yInc)
{
	int8_t xSign = (xInc <= 0)?-1:1;
	int8_t ySign = (yInc <= 0)?-1:1;
	while(xInc != 0 || yInc != 0) {
		if(xInc != 0) {
			playerX += xSign;
			xInc -= xSign;
		}
		if(yInc != 0) {
			playerY += ySign;
			yInc -= ySign;
		}
		coll_result res = collisionDetection(&playerX, &playerY, playerSize, maze, points);
		if(res.end == 1) {
			gameState = WIN_STATE;
		}
		if(res.points == 1) {
			++score;
		}
	}
	
	updateGhosts(ghosts, maze);
	for(uint8_t i = 0; i < GHOST_COUNT; ++i) {
		if(ghostPlayerCD(playerX, playerY, playerSize, ghosts[i])) {
			gameState = LOSE_STATE;
		}
	}
}

static void parseAccelData(int8_t* xInc, int8_t* yInc)
{
	*xInc = 0;
	*yInc = 0;
	uint8_t uX;
	uint8_t uY;
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		uX = (uint8_t) userX;
		uY = (uint8_t) userY;
	}
	if(uX >= 10 && uX <= 50) {
		*xInc = 1;
	} 
	else if(uX > 50 && uX < 128) {
		*xInc = 2;
	}
	else if(uX >= 128 && uX < 195) {
		*xInc = -2;
	}
	else if(uX >= 195 && uX <= 245) {
		*xInc = -1;
	}
	if(uY >= 2 && uY <= 12) {
		*yInc = -1;
	}
	else if(uY > 12 && uY <= 64) {
		*yInc = -2;
	}
	else if(uY >= 205 && uY < 235) {
		*yInc = 2;
	}
	else if(uY >= 235 && uY <= 245) {
		*yInc = 1;
	}
}


static void resetGameState(void) {
	playerX = 2;
	playerY = 2;
	score = 0;

	for(uint8_t i = 0; i < MAZE_WIDTH; ++i) {
		for(uint8_t j = 0; j < MAZE_HEIGHT; ++j) {
			maze[i][j].field = 0;
			points[i][j] = 1;
		}
	}
}

static void generateGhosts(void) {
	for(uint8_t i = 0; i < GHOST_COUNT; ++i) {
		ghost g;
		uint16_t rn = rand16();
		g.x = ((rn % (MAZE_WIDTH - GHOST_FREE_TILES)) + GHOST_FREE_TILES)* TILE_SIZE + 2;
		rn = rand16();
		g.y = ((rn % (MAZE_HEIGHT - GHOST_FREE_TILES)) + GHOST_FREE_TILES) * TILE_SIZE + 1;
		maze_tile tile = maze[g.x / TILE_SIZE][g.y / TILE_SIZE];
		if(tile.tile.freeRight) {
			g.direction = RIGHT;
		}
		else if(tile.tile.freeLeft) {
			g.direction = LEFT;
		}
		else if(tile.tile.freeTop) {
			g.direction = UP;
		}
		else if(tile.tile.freeBottom) {
			g.direction = DOWN;
		}
		ghosts[i] = g;
	}
}

/*********************
 *
 * Callbacks
 *
 *********************/

static void rcvButton(uint8_t wii, uint16_t buttonStates)
{
	if(buttonStates == 0x0008 && (gameState == WIN_STATE || gameState == LOSE_STATE)) {
		gameState = LOADING_STATE;
	}
}

static void rcvAccel(uint8_t wii, uint16_t x, uint16_t y, uint16_t z)
{
	accelData = 1;
	if(x > userX) {
		userX = x;
	}
	if(y > userY) {
		userY = y;
	}
	if(z > userZ) {
		userZ = z;
	}
}

static void conCallback(uint8_t wii, connection_status_t status)
{
	if(status == CONNECTED) {
		userConnected = 1;
		wiiUserSetAccel(0, 1, setAccelCallback);
		wiiUserSetLeds(0, 0x01, setLedsCallback);
	} else {
		userConnected = 0;
		wiiUserConnect(0, _mac, conCallback);
	}
}

static void setLedsCallback(uint8_t wii, error_t status)
{
} 

static void setAccelCallback(uint8_t wii, error_t status)
{
	if(status != SUCCESS) {
		wiiUserSetAccel(0, 1, setAccelCallback);
	}
}

static void randCb(uint8_t measure)
{
	rand_feed(measure & 0x01);
}
