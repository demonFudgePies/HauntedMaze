#include "mazeGen/mazeGen.h"
#include "ghost.h"

/**
 * @brief Stores the results of player-maze collision detection.
 */
typedef struct coll_result_t {
	uint8_t end;
	uint8_t points;
} coll_result;

/**
 * @brief Calculates player collisions with the maze and updates the player coordinates
 * so that there is no overlapping occuring. It also updates the points 2D array.
 *
 * @param playerX The x coordinate of the top left corner of the player.
 * @param playerY The y coordinate of the top left corner of the player.
 * @param maze The maze to check against.
 * @param points Binary map marking where there are points the player can still collect.
 * @return end is set if player is in the end tile. points is set if the player has any collected points.
 */ 
coll_result collisionDetection(int16_t* playerX, int16_t* playerY, uint8_t playerSize, 
									maze_tile maze[][MAZE_HEIGHT], uint8_t points[][MAZE_HEIGHT]);

/**
 * @brief Updates the ghost positions, and turns around corners in an almost random way. 
 * 
 * The actual update will depend on the position of the ghost, number and arrangement of the walls around it,
 * the direction of its motion, and a random variable. 
 * @param ghosts Array of length (GHOST_COUNT) containing all ghost information that needs to be updated.
 * @param maze The maze to use as a reference point.
 */
void updateGhosts(ghost ghosts[], maze_tile maze[][MAZE_HEIGHT]);

/**
 * @brief Collision detection routine between the player and a ghost.
 * @param playerX Player's x coordinete.
 * @param playerY Player's y coordinate.
 * @param playerSize Size of the player.
 * @param g Ghost to check against.
 * @return 1 if there was a collision, 0 otherwise.
 */
uint8_t ghostPlayerCD(int16_t playerX, int16_t playerY, uint8_t playerSize, ghost g);
