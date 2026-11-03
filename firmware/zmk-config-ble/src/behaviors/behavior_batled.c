/*
 * &batled: show the keyboard battery level on LED9 (1 s on a tap, or while held, max 10 s).
 * Runs on the keyboard that pressed the key (event-source locality), so it also works
 * when a dongle is the central (BLE or ESB): the dongle forwards it to the keyboard.
 * Copyright (c) 2026 Redwan. SPDX-License-Identifier: MIT
 */
#define DT_DRV_COMPAT kerf_behavior_batled

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#if IS_ENABLED(CONFIG_KERF_BATLED)
#include <kerf/led_split.h>
#endif

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int batled_pressed(struct zmk_behavior_binding *binding,
                          struct zmk_behavior_binding_event event) {
#if IS_ENABLED(CONFIG_KERF_BATLED)
    kerf_batled_show();
#endif
    return ZMK_BEHAVIOR_OPAQUE;
}

static int batled_released(struct zmk_behavior_binding *binding,
                           struct zmk_behavior_binding_event event) {
#if IS_ENABLED(CONFIG_KERF_BATLED)
    kerf_batled_release();
#endif
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api batled_api = {
    .binding_pressed = batled_pressed,
    .binding_released = batled_released,
    .locality = BEHAVIOR_LOCALITY_EVENT_SOURCE,
};

BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL,
                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &batled_api);

#endif
