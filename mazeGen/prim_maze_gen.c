/**
 * @brief Implements a maze generator based on Prim's algorithm.
 */

#include "mazeGen.h"
#include "../rand/rand.h"


// types of walls
typedef enum {
	LEFT_WALL,
	RIGHT_WALL,
	TOP_WALL,
	BOTTOM_WALL
} wall_t;

// entry in the wall list
typedef struct {
	wall_t wall;
	uint8_t x;
	uint8_t y;
} wlist_elem;

// list of all active walls
// there can never be more than MAZE_WIDTH * MAZE_HEIGHT * 2 walls in the list 
static wlist_elem wlist[MAZE_WIDTH * MAZE_HEIGHT * 2];
// end of the wall list
static uint32_t wlistEnd = 0;

static wlist_elem getRandomWall(void);
static void addWall(wall_t wall, uint8_t x, uint8_t y);
static void addCell(maze_tile tiles[][MAZE_HEIGHT], uint8_t x, uint8_t y);

// retrieves a random element from the wall list and removes it
static wlist_elem getRandomWall(void)
{
	uint16_t randIdx = rand16() % wlistEnd;
	wlist_elem result = wlist[randIdx];
	--wlistEnd;
	wlist[randIdx] = wlist[wlistEnd];
	return result;
}

// adds a wall to the wall list
static void addWall(wall_t wall, uint8_t x, uint8_t y)
{
	wlist_elem elem;
	elem.wall = wall;
	elem.x = x;
	elem.y = y;
	wlist[wlistEnd] = elem;
	++wlistEnd;
}

// adds all walls on cell to the wall list
static void addCell(maze_tile tiles[][MAZE_HEIGHT], uint8_t x, uint8_t y)
{
	addWall(LEFT_WALL, x, y);	
	addWall(RIGHT_WALL, x, y);	
	addWall(TOP_WALL, x, y);	
	addWall(BOTTOM_WALL, x, y);
	tiles[x][y].tile.visited = 1;
}

// generates a part of the maze, according to the interface provided in mazeGen.h
uint8_t generateMaze(maze_tile tiles[][MAZE_HEIGHT], uint16_t start, uint16_t length)
{
	uint8_t xInit = start / MAZE_WIDTH;
  	uint8_t yInit = start % MAZE_WIDTH;
  	
	if(length == 0) {
		return SUCCESS;
	}
	if(wlistEnd == 0) {
		addCell(tiles, xInit, yInit);
		tiles[xInit][yInit].tile.isStart = 1;
		tiles[xInit][yInit].tile.visited = 1;
  		tiles[MAZE_WIDTH - 1][MAZE_HEIGHT - 1].tile.isEnd = 1; 	// comment out this line and ucomment
  		//tiles[3][3].tile.isEnd = 1; 							// this one for the chance to see the end screen
	}
	
	for(uint16_t i = 0; i < length * 2 && wlistEnd > 0;) {
		wlist_elem welem = getRandomWall();
		uint8_t x = welem.x;
		uint8_t y = welem.y;
		if(welem.wall == LEFT_WALL) {
			if(x == 0 || tiles[x - 1][y].tile.visited) {
				continue;
			}
			addWall(LEFT_WALL, x - 1, y);
			addWall(TOP_WALL, x - 1, y);
			addWall(BOTTOM_WALL, x - 1, y);
			tiles[x][y].tile.freeLeft = 1;
			tiles[x - 1][y].tile.freeRight = 1;
			tiles[x - 1][y].tile.visited = 1;
			++i;
		}
		else if(welem.wall == RIGHT_WALL) {
			if(x == MAZE_WIDTH - 1 || tiles[x + 1][y].tile.visited) {
				continue;
			}
			addWall(TOP_WALL, x + 1, y);
			addWall(BOTTOM_WALL, x + 1, y);
			addWall(RIGHT_WALL, x + 1, y);
			tiles[x][y].tile.freeRight = 1;
			tiles[x + 1][y].tile.freeLeft = 1;
			tiles[x + 1][y].tile.visited = 1;
			++i;
		}
		else if(welem.wall == TOP_WALL) {
			if(y == 0 || tiles[x][y - 1].tile.visited) {
				continue;
			}
			addWall(LEFT_WALL, x, y - 1);
			addWall(RIGHT_WALL, x, y - 1);
			addWall(TOP_WALL, x, y - 1);
			tiles[x][y].tile.freeTop = 1;
			tiles[x][y - 1].tile.freeBottom = 1;
			tiles[x][y - 1].tile.visited = 1;
			++i;
		}
		else if(welem.wall == BOTTOM_WALL) {
			if(y == MAZE_HEIGHT - 1 || tiles[x][y + 1].tile.visited) {
				continue;
			}
			addWall(LEFT_WALL, x, y + 1);
			addWall(RIGHT_WALL, x, y + 1);
			addWall(BOTTOM_WALL, x, y + 1);	
			tiles[x][y].tile.freeBottom = 1;
			tiles[x][y + 1].tile.freeTop = 1;	
			tiles[x][y + 1].tile.visited = 1;
			++i;
		}
	}
	return SUCCESS;
}


