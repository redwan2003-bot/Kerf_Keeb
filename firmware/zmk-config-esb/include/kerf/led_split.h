#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <zephyr/drivers/led_strip.h>
/* Set status pixel <idx> (0 = first pixel after the underglow part) and resend the chain.
 * blank_main clears the underglow pixels too (used when underglow is off). */
int kerf_led_split_set_extra(uint8_t idx, struct led_rgb color, bool blank_main);
/* Show the battery level on the indicator LED (min CONFIG_KERF_BATLED_SHOW_MS, or while held). */
void kerf_batled_show(void);
/* &batled key released: ends a held display. */
void kerf_batled_release(void);
