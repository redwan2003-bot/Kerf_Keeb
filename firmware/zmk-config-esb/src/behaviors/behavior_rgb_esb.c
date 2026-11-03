/*
 * &rgb_esb: underglow commands routed to the keyboard that pressed the key.
 * On the dongle (central) the command goes over ESB to that peripheral,
 * where it is handed to the stock &rgb_ug behaviour.
 * Copyright (c) 2026 Redwan. SPDX-License-Identifier: MIT
 */
#define DT_DRV_COMPAT kerf_behavior_rgb_esb

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/keymap.h>
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
#include <zmk/split/central.h>
#endif

/* ESB pipe of the keyboard, used when the command did not come from a key on it
 * (encoder rotations: ZMK's sensor-rotate marks them as local to the dongle). */
#if DT_NODE_EXISTS(DT_NODELABEL(lp63_keyboard))
#define RGB_ESB_DEFAULT_PIPE DT_PROP(DT_NODELABEL(lp63_keyboard), pipe)
#else
#define RGB_ESB_DEFAULT_PIPE 0
#endif

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int rgb_esb_pressed(struct zmk_behavior_binding *binding,
                           struct zmk_behavior_binding_event event) {
#if IS_ENABLED(CONFIG_ZMK_RGB_UNDERGLOW)
    struct zmk_behavior_binding fwd = {
        .behavior_dev = "rgb_ug",
        .param1 = binding->param1,
        .param2 = binding->param2,
    };
    return zmk_behavior_invoke_binding(&fwd, event, true);
#elif IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    /* dongle has no LED strip: a key on the keyboard never lands here (EVENT_SOURCE locality
     * sends it straight to the keyboard), but encoder steps do, so pass them on. */
    zmk_split_central_invoke_behavior(RGB_ESB_DEFAULT_PIPE, binding, event, true);
    return ZMK_BEHAVIOR_OPAQUE;
#else
    return ZMK_BEHAVIOR_OPAQUE;
#endif
}

static int rgb_esb_released(struct zmk_behavior_binding *binding,
                            struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api rgb_esb_api = {
    .binding_pressed = rgb_esb_pressed,
    .binding_released = rgb_esb_released,
    .locality = BEHAVIOR_LOCALITY_EVENT_SOURCE,
};

BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL,
                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &rgb_esb_api);

#endif
