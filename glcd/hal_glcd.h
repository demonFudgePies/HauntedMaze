#include <avr/io.h>

/**
 * @brief Initializes the hardware abstraction layer for the glcd.
 */
void halGlcdInit(void);

/**
 * @brief sets the current address to the xCol-th column the the yPage-th page.
 */
uint8_t halGlcdSetAddress(const uint8_t xCol, const uint8_t yPage);

/**
 * @brief writes data to the glcd ram to current address
 */
uint8_t halGlcdWriteData(const uint8_t data);

/**
 * @brief reads data to glcd ram from current address
 */
uint8_t halGlcdReadData(void);

/**
 * @brief sets the y shift of the glcd
 */
void halGlcdSetYShift(uint8_t y);
