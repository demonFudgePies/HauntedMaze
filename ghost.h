#ifndef __GAME_STATES__
#define __GAME_STATES__

// width and height of a ghost in pixels
#define GHOST_W 4
#define GHOST_H 6

// number of ghosts in the map
#define GHOST_COUNT 16

/**
 * @brief The direction where the ghost will try to go.
 */
typedef enum {
	DOWN,
	LEFT,
	RIGHT,
	UP
} ghost_dir_t;

/**
 * @brief Contains all information needed for one ghost.
 */
typedef struct ghost_t {
	uint8_t x;
	uint8_t y;
	ghost_dir_t direction;
} ghost;

#endif
