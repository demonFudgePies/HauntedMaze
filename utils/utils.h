#ifndef NULL
#define NULL ((void *) 0)
#endif

#ifndef TRUE
#define TRUE (0==0)
#endif

#ifndef FALSE
#define FALSE (!TRUE)
#endif

#define set_port_bits(port,mask) ((port) |= (mask))
#define clear_port_bits(port,mask) ((port) &= (~(mask)))

#define ABS(x) ((x >= 0)?x:-x)
#define SGN(x) ((x > 0)?1:((x<0)?-1:0))

/**
 * @brief Sets the bits in mask as output. Other bits are untouched.
 */
void setAsOut(volatile uint8_t *port, volatile uint8_t *ddr, uint8_t mask);

/**
 * @brief Sets the bits in mask as input with pull up. Other bits are untouched.
 */
void setAsIn(volatile uint8_t *port, volatile uint8_t *ddr, uint8_t mask);

/**
 * @brief Sets the bits in mask as input without. Other bits are untouched.
 */
void setAsInNoPullUp(volatile uint8_t *port, volatile uint8_t *ddr, uint8_t mask);
