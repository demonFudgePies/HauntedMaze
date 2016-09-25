#include "mazeGen/mazeGen.h"
#include "ghost.h"

/**
 * @brief Initializes the rendered.
 *
 * Since the renderer needs to track the player position for most actions,
 * pointers to position data are given here. These will not be used for 
 * changing the player position values from within.
 * @param playerX Pointer to player's x position
 * @param playerY Pointer to player's y position
 * @param psize The player size
 */
void rendererInit(int16_t* playerX, int16_t* playerY, uint8_t psize);

/**
 * @brief Updates the internal camera according to the player position
 */
void updateCamera(void);

/**
 * @brief Draws the player into the frame buffer.
 */
void renderPlayer(void);

/**
 * @brief Draws the visible part of the maze and the points into the frame buffer.
 *
 * @param maze The maze to be drawn.
 * @param points 2D array containing the information which points are still visible.
 */
void renderMaze(maze_tile maze[][MAZE_HEIGHT], uint8_t points[][MAZE_HEIGHT]);

/**
 * @brief Draws all the ghosts on screen.
 *
 * @param ghosts Array size of ghostCount containing all ghosts to be drawn.
 * @param ghostCount How many ghosts to draw. Could potentially be used for
 * killing off ghosts.
 */
void renderGhosts(ghost ghosts[], uint8_t ghostCount);

/**
 * @brief Flushes the framebuffer to the screen
 */
void endRender(void);

/**
 * @brief Prepares the framebuffer for drawing.
 */
void startRender(void);

/**
 * @brief Draws the end screen into the framebuffer.
 *
 * @param won Function displays win message if won = 1, and lose message otherwise.
 * @param score Player's score to be drawn.
 */
void drawEndScreen(uint8_t won, uint16_t score);

/**
 * @brief Draws the opening screen image into the framebuffer.
 */
void drawStartScreen(void);

/**
 * @brief Draws the "waiting for wiimote" screen" into the framebuffer.
 */
void drawConnectingScreen(void);

/**
 * @brief Updates all animations according to predetermined prescalars.
 */
void updateAnimations(void);

/**
 * @brief Updates the animation for the start screen.
 * 
 * @return 1 if the animation is completed, 0 if there are more frames.
 */
uint8_t updateStartScreen(void);
