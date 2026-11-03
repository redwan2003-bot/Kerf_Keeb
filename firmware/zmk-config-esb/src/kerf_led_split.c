/*
 * kerf,led-strip-split: shows the first <chain-length> pixels of a real WS2812 chain to
 * ZMK underglow, and keeps the remaining pixel(s) for status use (LP63: pixel 9 = LED9,
 * the battery indicator right of the Up arrow).
 * Copyright (c) 2026 Redwan. SPDX-License-Identifier: MIT
 */
#define DT_DRV_COMPAT kerf_led_strip_split

#include <string.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#if __has_include(<zephyr/version.h>)
#include <zephyr/version.h>
#else
#include <version.h>
#endif
#include <zephyr/drivers/led_strip.h>
#include <kerf/led_split.h>

#define HW_NODE DT_INST_PHANDLE(0, strip)
#define HW_LEN DT_PROP(HW_NODE, chain_length)
#define UG_LEN DT_INST_PROP(0, chain_length)
BUILD_ASSERT(HW_LEN > UG_LEN, "real strip must be longer than the underglow part");

static const struct device *const hw = DEVICE_DT_GET(HW_NODE);
static struct led_rgb main_px[UG_LEN];
static struct led_rgb extra_px[HW_LEN - UG_LEN];
static struct led_rgb tx[HW_LEN];
static K_MUTEX_DEFINE(lock);

static int send_locked(void) {
    memcpy(tx, main_px, sizeof(main_px));
    memcpy(&tx[UG_LEN], extra_px, sizeof(extra_px));
    return led_strip_update_rgb(hw, tx, HW_LEN);
}

static int split_update_rgb(const struct device *dev, struct led_rgb *pixels, size_t n) {
    if (n > UG_LEN) {
        n = UG_LEN;
    }
    k_mutex_lock(&lock, K_FOREVER);
    memcpy(main_px, pixels, n * sizeof(struct led_rgb));
    int rc = send_locked();
    k_mutex_unlock(&lock);
    return rc;
}

static int split_update_channels(const struct device *dev, uint8_t *ch, size_t n) {
    return -ENOTSUP;
}

#if KERNEL_VERSION_NUMBER >= ZEPHYR_VERSION(3, 7, 0)
static size_t split_length(const struct device *dev) { return UG_LEN; }
#endif

int kerf_led_split_set_extra(uint8_t idx, struct led_rgb color, bool blank_main) {
    if (idx >= ARRAY_SIZE(extra_px)) {
        return -EINVAL;
    }
    k_mutex_lock(&lock, K_FOREVER);
    extra_px[idx] = color;
    if (blank_main) {
        memset(main_px, 0, sizeof(main_px));
    }
    int rc = send_locked();
    k_mutex_unlock(&lock);
    return rc;
}

static int split_init(const struct device *dev) { return device_is_ready(hw) ? 0 : -ENODEV; }

static const struct led_strip_driver_api split_api = {
    .update_rgb = split_update_rgb,
    .update_channels = split_update_channels,
#if KERNEL_VERSION_NUMBER >= ZEPHYR_VERSION(3, 7, 0)
    .length = split_length,
#endif
};

DEVICE_DT_INST_DEFINE(0, split_init, NULL, NULL, NULL, POST_KERNEL,
                      CONFIG_KERF_LED_SPLIT_INIT_PRIORITY, &split_api);
