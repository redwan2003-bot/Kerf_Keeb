/*
 * LP63 battery indicator (LED9, WS2812B-Mini 3535 on the top side (hand-soldered), last pixel of the underglow chain).
 *  - &batled key: shows the charge (green > 50 %, yellow 20-50 %, red < 20 %) for 1 s on a tap,
 *    or for as long as the key is held (max 10 s).
 *  - low battery (<= CONFIG_KERF_BATLED_LOW_PERCENT, on battery only): three short red
 *    blinks, at most once every CONFIG_KERF_BATLED_LOW_REPEAT_MIN minutes.
 * The LED rail (Q1, EXT_POWER on D20) is switched on only while the LED is lit and off
 * again afterwards (unless underglow is using it), so the indicator draws 0 mA when idle.
 * Copyright (c) 2026 Redwan. SPDX-License-Identifier: MIT
 */
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <drivers/ext_power.h>
#include <zmk/battery.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/workqueue.h>
#if IS_ENABLED(CONFIG_ZMK_RGB_UNDERGLOW)
#include <zmk/rgb_underglow.h>
#endif
#if IS_ENABLED(CONFIG_ZMK_USB)
#include <zmk/usb.h>
#endif
#include <kerf/led_split.h>

LOG_MODULE_REGISTER(kerf_batled, CONFIG_ZMK_LOG_LEVEL);

#define BATLED_BRT CONFIG_KERF_BATLED_BRIGHTNESS
/* Red is shown when the cell is low (3.3-3.7 V). Red needs only ~2 V, so it keeps full
 * current there; a higher PWM level just makes it clearly visible. */
#define BATLED_LOW_BRT CONFIG_KERF_BATLED_LOW_BRIGHTNESS
/* Yellow: weight it toward red (2:1) so it reads as yellow, not lime, on a WS2812. */
#define BATLED_YEL_R MIN(255, (BATLED_BRT * 3) / 2)
#define BATLED_YEL_G ((BATLED_BRT * 3) / 4)
static const struct device *const ext_pwr = DEVICE_DT_GET_OR_NULL(DT_INST(0, zmk_ext_power_generic));

static bool we_powered;
static int blinks_left; /* >0: low-battery blink sequence running (counts on/off phases) */
static bool showing;
static bool held;       /* &batled key still pressed */
static bool min_done;   /* minimum display time has passed */
static int64_t show_start;
static int64_t last_low_flash = -1;

static void work_handler(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(batled_work, work_handler);

static bool underglow_on(void) {
#if IS_ENABLED(CONFIG_ZMK_RGB_UNDERGLOW)
    bool on = false;
    zmk_rgb_underglow_get_state(&on);
    return on;
#else
    return false;
#endif
}

static void power_up(void) {
    if (ext_pwr && device_is_ready(ext_pwr) && ext_power_get(ext_pwr) <= 0) {
        ext_power_enable(ext_pwr);
        we_powered = true;
        k_msleep(5); /* LED rail + WS2812 power-on */
    }
}

static void power_down(void) {
    if (we_powered && !underglow_on() && ext_pwr) {
        ext_power_disable(ext_pwr);
    }
    we_powered = false;
}

static void set_led(struct led_rgb c) { kerf_led_split_set_extra(0, c, !underglow_on()); }

static struct led_rgb level_color(uint8_t soc) {
    if (soc > 50) {
        return (struct led_rgb){.r = 0, .g = BATLED_BRT, .b = 0};
    } else if (soc >= 20) {
        return (struct led_rgb){.r = BATLED_YEL_R, .g = BATLED_YEL_G, .b = 0};
    }
    return (struct led_rgb){.r = BATLED_LOW_BRT, .g = 0, .b = 0};
}

static void work_handler(struct k_work *work) {
    if (showing) {
        int64_t lit = k_uptime_get() - show_start;
        if (held && lit < CONFIG_KERF_BATLED_MAX_MS) {
            /* minimum time is over but the key is still held: stay on until release */
            min_done = true;
            k_work_schedule_for_queue(zmk_workqueue_lowprio_work_q(), &batled_work,
                                      K_MSEC(CONFIG_KERF_BATLED_MAX_MS - lit));
            return;
        }
        showing = false;
        held = false;
        set_led((struct led_rgb){0});
        power_down();
        return;
    }
    if (blinks_left > 0) {
        blinks_left--;
        bool on = (blinks_left % 2) == 1;
        set_led(on ? (struct led_rgb){.r = BATLED_LOW_BRT, .g = 0, .b = 0} : (struct led_rgb){0});
        if (blinks_left > 0) {
            k_work_schedule_for_queue(zmk_workqueue_lowprio_work_q(), &batled_work, K_MSEC(150));
        } else {
            power_down();
        }
    }
}

static void show_handler(struct k_work *work) {
    uint8_t soc = zmk_battery_state_of_charge();
    blinks_left = 0;
    power_up();
    set_led(level_color(soc));
    showing = true;
    held = true;
    min_done = false;
    show_start = k_uptime_get();
    k_work_reschedule_for_queue(zmk_workqueue_lowprio_work_q(), &batled_work,
                                K_MSEC(CONFIG_KERF_BATLED_SHOW_MS));
    LOG_INF("battery %u%%", soc);
}
static K_WORK_DEFINE(show_work, show_handler);

void kerf_batled_show(void) { k_work_submit_to_queue(zmk_workqueue_lowprio_work_q(), &show_work); }

static void release_handler(struct k_work *work) {
    held = false;
    if (showing && min_done) { /* held past the minimum: switch off right away */
        k_work_reschedule_for_queue(zmk_workqueue_lowprio_work_q(), &batled_work, K_NO_WAIT);
    }
}
static K_WORK_DEFINE(release_work, release_handler);

void kerf_batled_release(void) {
    k_work_submit_to_queue(zmk_workqueue_lowprio_work_q(), &release_work);
}

static void low_handler(struct k_work *work) {
    if (showing || blinks_left > 0) {
        return;
    }
    power_up();
    blinks_left = 6; /* on/off x3 */
    k_work_schedule_for_queue(zmk_workqueue_lowprio_work_q(), &batled_work, K_NO_WAIT);
}
static K_WORK_DEFINE(low_work, low_handler);

static int batled_listener(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *ev = as_zmk_battery_state_changed(eh);
    if (ev == NULL || ev->state_of_charge == 0 ||
        ev->state_of_charge > CONFIG_KERF_BATLED_LOW_PERCENT) {
        return ZMK_EV_EVENT_BUBBLE;
    }
#if IS_ENABLED(CONFIG_ZMK_USB)
    if (zmk_usb_is_powered()) {
        return ZMK_EV_EVENT_BUBBLE; /* charging: no warning */
    }
#endif
    int64_t now = k_uptime_get();
    if (last_low_flash >= 0 &&
        now - last_low_flash < (int64_t)CONFIG_KERF_BATLED_LOW_REPEAT_MIN * 60 * 1000) {
        return ZMK_EV_EVENT_BUBBLE;
    }
    last_low_flash = now;
    k_work_submit_to_queue(zmk_workqueue_lowprio_work_q(), &low_work);
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(kerf_batled, batled_listener);
ZMK_SUBSCRIPTION(kerf_batled, zmk_battery_state_changed);
