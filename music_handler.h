#include <inttypes.h>

/**
 * @brief Initializes everything needed to play music.
 */
void musicInit(void);

/**
 * @brief Sets the volume according to measure using a linear loudness scale.
 *
 * This function is given as a callback to the ADC. It technically only sets
 * an internal variable, and the volume will be changed during the next call to
 * musicBckg()
 * @param measure The volume will be updated according to this parametar.
 */
void volumeCb(uint8_t measure);

/**
 * @brief A background task which loads an amount of music and sends it to the mp3 decoder.
 */
void musicBckg(void);
