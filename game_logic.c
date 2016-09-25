#include "game_logic.h"
#include "rand/rand.h"

// 2D cartesian point
typedef struct Point_t {
	int32_t x;
	int32_t y;
} Point;

/**
 * @brief Helper function determining whether the ghost can/should make a turn.
 * @param tile The tile in which the ghost is located.
 * @return 1 if the ghost should turn, 0 otherwise.
 */
static uint8_t shouldTurn(maze_tile tile);

/**
 * @brief Chooses a random movement direction for the ghost.
 * @param g The ghost to update.
 * @param the tile where the ghost is located.
 */
static void chooseRandDir(ghost *g, maze_tile tile);

uint8_t ghostPlayerCD(int16_t playerX, int16_t playerY, uint8_t playerSize, ghost g) {
	if(playerX < g.x + GHOST_W &&
		playerX + playerSize > g.x &&
		playerY < g.y + GHOST_H &&
		playerY + playerSize > g.y) {
		return 1;
	}
	return 0;
}

// per pixel collision detection
// when this function was written, only God and I knew how it works.
// now, only God does.
coll_result collisionDetection(int16_t* playerX, int16_t* playerY, uint8_t playerSize, 
									maze_tile maze[][MAZE_HEIGHT], uint8_t points[][MAZE_HEIGHT])
{
	Point tlTile = {*playerX / TILE_SIZE, *playerY / TILE_SIZE};
	Point trTile = {(*playerX + playerSize - 1) / TILE_SIZE, *playerY / TILE_SIZE};
	Point blTile = {*playerX / TILE_SIZE, (*playerY + playerSize - 1) / TILE_SIZE};
	Point brTile = {(*playerX + playerSize - 1) / TILE_SIZE, (*playerY + playerSize - 1) / TILE_SIZE};
	
	coll_result result = {0, 0};
	
	if(maze[tlTile.x][tlTile.y].tile.isEnd || 
		maze[trTile.x][trTile.y].tile.isEnd ||
		maze[blTile.x][blTile.y].tile.isEnd ||
		maze[brTile.x][brTile.y].tile.isEnd)
	{
		result.end = 1;
	}
	
	// CHECKING OBSTICLES TO THE BOTTOM
	if( (uint8_t) ((*playerY + playerSize - 1) % TILE_SIZE) == 0) { // if entered the next field
		if(tlTile.x != trTile.x) { // no need to check a wall, because there is always a wall inbetween
			--*playerY;
		}
	} else if((uint8_t)(*playerY + playerSize - 1) % TILE_SIZE == 7) {
		if(!maze[tlTile.x][tlTile.y].tile.freeBottom || !maze[trTile.x][trTile.y].tile.freeBottom) {
			--*playerY;	
		}
	}
	
	// CHECKING OBSTICLES TO THE RIGHT
	if( (uint8_t) ((*playerX + playerSize - 1) % TILE_SIZE) == 0) {
		if(tlTile.y != blTile.y) {
			--*playerX;		
		}
	} else if((uint8_t)(*playerX + playerSize - 1) % TILE_SIZE == 7) {
		if(!maze[tlTile.x][tlTile.y].tile.freeRight || !maze[blTile.x][blTile.y].tile.freeRight) {
			--*playerX;		
		}
	}
	
	// CHECKING OBSTICLES TO THE TOP
	if((uint8_t)(*playerY % TILE_SIZE) == 7) {
		if(tlTile.x != trTile.x) {
			++*playerY;
		}
	} else if((uint8_t)(*playerY % TILE_SIZE) == 0) {
		if(!maze[tlTile.x][tlTile.y].tile.freeTop || !maze[trTile.x][trTile.y].tile.freeTop) {
			++*playerY;
		}
	}
	
	// CHECKING OBSTICLES TO THE LEFT
	if((uint8_t)(*playerX % TILE_SIZE) == 7) {
		if(tlTile.y != blTile.y) {
			++*playerX;	
		}
	} else if((uint8_t)(*playerX % TILE_SIZE) == 0) {
		if(!maze[tlTile.x][tlTile.y].tile.freeLeft || !maze[blTile.x][blTile.y].tile.freeLeft) {
			++*playerX;
		}
	}
	
	// all 4 quadrants are the same are the only way of him eating a point because of the chosen sizes
	if(tlTile.x == brTile.x && tlTile.y == brTile.y && points[tlTile.x][tlTile.y] == 1) { 
		points[tlTile.x][tlTile.y] = 0;
		result.points = 1;
	}
	return result;
}


void updateGhosts(ghost ghosts[], maze_tile maze[][MAZE_HEIGHT])
{
	for(uint8_t i = 0; i < GHOST_COUNT; ++i) {
		maze_tile currTile = maze[ghosts[i].x / TILE_SIZE][ghosts[i].y / TILE_SIZE];
		if(ghosts[i].x % TILE_SIZE == 2 && ghosts[i].y % TILE_SIZE == 1 
			&& shouldTurn(currTile) == 1) {
			chooseRandDir(&ghosts[i], currTile);
		}
		switch(ghosts[i].direction) {
			case RIGHT:
				ghosts[i].x++;
				break;
			case LEFT:
				ghosts[i].x--;
				break;
			case UP:
				ghosts[i].y--;
				break;
			case DOWN:
				ghosts[i].y++;
				break;
		}
	}
}


static uint8_t shouldTurn(maze_tile tile)
{
	uint8_t count = !tile.tile.freeRight + !tile.tile.freeLeft 
			+ !tile.tile.freeTop + !tile.tile.freeBottom;
	if((count < 3 && !tile.tile.freeRight && !tile.tile.freeLeft) ||
		(count < 3 && !tile.tile.freeTop && !tile.tile.freeBottom)) {
		return 0;
	}
	return 1;
}

static void chooseRandDir(ghost *g, maze_tile tile) {
	uint8_t oDir = g->direction;
	uint8_t pickedNew = 0;
	uint16_t rn = rand16();
	for(uint8_t i = 0; i < 16; ++i) {
		g->direction = rn % 4;
		rn >>= 1;
		if(g->direction == RIGHT && tile.tile.freeRight) {
			pickedNew = 1;
			break;
		}
		else if(g->direction == DOWN && tile.tile.freeBottom) {
			pickedNew = 1;
			break;
		}
		else if(g->direction == LEFT && tile.tile.freeLeft) {
			pickedNew = 1;
			break;
		}
		else if(g->direction == UP && tile.tile.freeTop) {
			pickedNew = 1;
			break;
		}
	}
	uint8_t wallCount = !tile.tile.freeRight + !tile.tile.freeLeft 
			+ !tile.tile.freeTop + !tile.tile.freeBottom;
	if(pickedNew == 0 && wallCount == 3) {
		g->direction = ~oDir;
	}
}
