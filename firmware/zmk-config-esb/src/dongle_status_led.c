/*
 * kerf dongle RGB status LED
 * Copyright (c) 2026 Redwan. SPDX-License-Identifier: MIT
 *
 * One colour per job:
 * Works with the Bluetooth link and with the Nordic ESB link.
 *
 *   BLUE  - keyboard link: fast blink while searching (never connected since boot),
 *           solid for 2 s when the keyboard connects, then off,
 *           slow blink if the keyboard link is lost.
 *   GREEN - Caps Lock on the host (solid while active).
 *   RED   - keyboard battery at or below CONFIG_KERF_DONGLE_LOW_BATTERY_PERCENT:
 *           a short flash every 2 s.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE)
#include <zephyr/bluetooth/conn.h>
#endif
#include <zephyr/logging/log.h>

#include <zmk/event_manager.h>
#include <zmk/events/hid_indicators_changed.h>
#include <zmk/events/battery_state_changed.h>
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ESB)
#include <zmk/events/split_esb_peripheral_changed.h>
#endif

#define KERF_BATTERY_EVENTS                                                                        \
    (IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING) ||                            \
     (IS_ENABLED(CONFIG_ZMK_SPLIT_ESB) && IS_ENABLED(CONFIG_ZMK_BATTERY_REPORTING)))

LOG_MODULE_REGISTER(kerf_led, CONFIG_ZMK_LOG_LEVEL);

static const struct gpio_dt_spec led_red = GPIO_DT_SPEC_GET(DT_ALIAS(led_red), gpios);
static const struct gpio_dt_spec led_green = GPIO_DT_SPEC_GET(DT_ALIAS(led_green), gpios);
static const struct gpio_dt_spec led_blue = GPIO_DT_SPEC_GET(DT_ALIAS(led_blue), gpios);

#define HID_CAPS_LOCK_BIT BIT(1) /* HID LED usage 0x02 */

enum link_state { LINK_SEARCHING, LINK_JUST_CONNECTED, LINK_CONNECTED, LINK_LOST };

static enum link_state link = LINK_SEARCHING;
static int64_t connected_at;
static bool caps_on;
static bool battery_low;

static void set(const struct gpio_dt_spec *led, bool on) { gpio_pin_set_dt(led, on ? 1 : 0); }

static void tick_handler(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(tick_work, tick_handler);

#define TICK_MS 50

static void tick_handler(struct k_work *work) {
    int64_t now = k_uptime_get();

    /* Blue: keyboard link */
    switch (link) {
    case LINK_SEARCHING: /* fast blink: 100 ms on / 100 ms off */
        set(&led_blue, (now / 100) % 2 == 0);
        break;
    case LINK_JUST_CONNECTED: /* solid 2 s, then off */
        set(&led_blue, true);
        if (now - connected_at >= 2000) {
            link = LINK_CONNECTED;
        }
        break;
    case LINK_CONNECTED:
        set(&led_blue, false);
        break;
    case LINK_LOST: /* slow blink: 150 ms on every 1 s */
        set(&led_blue, (now % 1000) < 150);
        break;
    }

    /* Green: Caps Lock */
    set(&led_green, caps_on);

    /* Red: keyboard battery low, 100 ms flash every 2 s */
    set(&led_red, battery_low && (now % 2000) < 100);

    k_work_reschedule(&tick_work, K_MSEC(TICK_MS));
}

static void link_up(void) {
    link = LINK_JUST_CONNECTED;
    connected_at = k_uptime_get();
}

static void link_down(void) {
    link = LINK_LOST;
    battery_low = false; /* unknown until the keyboard reports again */
}

#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE)
/* The dongle is BLE central only towards the keyboard; host links are peripheral role. */
static bool is_keyboard_link(struct bt_conn *conn) {
    struct bt_conn_info info;
    if (bt_conn_get_info(conn, &info) < 0) {
        return false;
    }
    return info.role == BT_CONN_ROLE_CENTRAL;
}

static void on_connected(struct bt_conn *conn, uint8_t err) {
    if (err || !is_keyboard_link(conn)) {
        return;
    }
    link_up();
}

static void on_disconnected(struct bt_conn *conn, uint8_t reason) {
    if (!is_keyboard_link(conn)) {
        return;
    }
    link_down();
}

BT_CONN_CB_DEFINE(kerf_led_conn_cb) = {
    .connected = on_connected,
    .disconnected = on_disconnected,
};

#endif /* CONFIG_ZMK_SPLIT_BLE */

static int kerf_led_listener(const zmk_event_t *eh) {
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ESB)
    const struct zmk_split_esb_peripheral_changed *esb = as_zmk_split_esb_peripheral_changed(eh);
    if (esb) {
        if (esb->connected) {
            link_up();
        } else {
            link_down();
        }
        return ZMK_EV_EVENT_BUBBLE;
    }
#endif

    const struct zmk_hid_indicators_changed *ind = as_zmk_hid_indicators_changed(eh);
    if (ind) {
        caps_on = (ind->indicators & HID_CAPS_LOCK_BIT) != 0;
        return ZMK_EV_EVENT_BUBBLE;
    }

#if KERF_BATTERY_EVENTS
    const struct zmk_peripheral_battery_state_changed *bat =
        as_zmk_peripheral_battery_state_changed(eh);
    if (bat) {
        battery_low = bat->state_of_charge > 0 &&
                      bat->state_of_charge <= CONFIG_KERF_DONGLE_LOW_BATTERY_PERCENT;
    }
#endif
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(kerf_led, kerf_led_listener);
ZMK_SUBSCRIPTION(kerf_led, zmk_hid_indicators_changed);
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ESB)
ZMK_SUBSCRIPTION(kerf_led, zmk_split_esb_peripheral_changed);
#endif
#if KERF_BATTERY_EVENTS
ZMK_SUBSCRIPTION(kerf_led, zmk_peripheral_battery_state_changed);
#endif

static int kerf_led_init(void) {
    const struct gpio_dt_spec *leds[] = {&led_red, &led_green, &led_blue};
    for (int i = 0; i < ARRAY_SIZE(leds); i++) {
        if (!gpio_is_ready_dt(leds[i])) {
            LOG_ERR("status LED %d not ready", i);
            return -ENODEV;
        }
        gpio_pin_configure_dt(leds[i], GPIO_OUTPUT_INACTIVE);
    }
    k_work_reschedule(&tick_work, K_MSEC(TICK_MS));
    return 0;
}

SYS_INIT(kerf_led_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
