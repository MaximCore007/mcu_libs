#ifndef ADDRESSABLELEDS_ADDR_LEDS_H_
#define ADDRESSABLELEDS_ADDR_LEDS_H_

#include "main.h"

// Number of LEDs in strip
#define NUMBER_OF_LEDS 12U

/*
 * The data transfer time for WS2812 is (LOG_1 + LOG_0) = 1,25us ± 600ns
 */
#define LOG_HIGH_TIME_SLOT	39	// 0.70us for '1' at 800kHz
#define LOG_LOW_TIME_SLOT	18	// 0.35us for '0' at 800kHz
#define RESET_TIME_SLOT		50	// 50us reset time slot

/*
 * The WS2812 has a data composition of 24 bits with the order of GRB
 */
#define BITS_PER_LED 24U
#define LED_R 1
#define LED_G 0
#define LED_B 2

void set_color(uint8_t led, uint8_t r, uint8_t g, uint8_t b);
int8_t leds_update(void);

#endif /* ADDRESSABLELEDS_ADDR_LEDS_H_ */
