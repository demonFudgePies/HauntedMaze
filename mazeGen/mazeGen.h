#ifndef __MAZEGEN_H__
#define __MAZEGEN_H__

#include <avr/pgmspace.h>
#include "util.h"

#define MAZE_WIDTH (32)
#define MAZE_HEIGHT (16)

#define TILE_SIZE (8)

typedef union maze_tile_t
{
  struct {
    uint8_t freeLeft:    1;  // set if there is no wall on the left
    uint8_t freeRight:   1;  // ... right
    uint8_t freeTop:     1;  // ... top
    uint8_t freeBottom:  1;  // ... bottom
    uint8_t visited:     1;  // set if already visited in maze generation
    uint8_t isStart:     1;  // set if tile is start
    uint8_t isEnd:       1;  // set if tile is end
  } tile;
  uint8_t field;
} maze_tile;

uint8_t generateMaze(maze_tile tiles[][MAZE_HEIGHT], uint16_t start, uint16_t length);

#endif
