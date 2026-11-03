/*
 * nice!view status screen for the LP63 keyboard when it talks to the dongle
 * over Nordic ESB instead of Bluetooth. Shows link state, RSSI, radio
 * channel, delivery cost and battery. Refreshed once a second.
 * Copyright (c) 2026 Redwan. SPDX-License-Identifier: MIT
 */
#include <stdio.h>
#include <zephyr/kernel.h>
#include <lvgl.h>
#include <zmk/display.h>
#include <zmk/battery.h>
#include <zmk_split_esb.h>

static lv_obj_t *lbl_title, *lbl_link, *lbl_radio, *lbl_batt;

static void refresh(lv_timer_t *t) {
    ARG_UNUSED(t);
    struct zmk_split_esb_status st;
    char buf[40];

    zmk_split_esb_get_status(&st);

    lv_label_set_text(lbl_link, st.searching ? LV_SYMBOL_CLOSE " searching"
                                             : LV_SYMBOL_WIFI " dongle");

    snprintf(buf, sizeof(buf), "%d dBm  ch%u  x%u.%u", st.rssi_dbm, st.channel,
             st.attempts_ewma_x10 / 10, st.attempts_ewma_x10 % 10);
    lv_label_set_text(lbl_radio, st.searching ? "-" : buf);

    uint8_t soc = zmk_battery_state_of_charge();
    const char *sym = soc > 75   ? LV_SYMBOL_BATTERY_FULL
                      : soc > 50 ? LV_SYMBOL_BATTERY_3
                      : soc > 25 ? LV_SYMBOL_BATTERY_2
                      : soc > 10 ? LV_SYMBOL_BATTERY_1
                                 : LV_SYMBOL_BATTERY_EMPTY;
    snprintf(buf, sizeof(buf), "%s %u%%", sym, soc);
    lv_label_set_text(lbl_batt, buf);
}

static lv_obj_t *mk_label(lv_obj_t *parent, const lv_font_t *font, int y) {
    lv_obj_t *l = lv_label_create(parent);
    lv_obj_set_style_text_font(l, font, 0);
    lv_obj_align(l, LV_ALIGN_TOP_LEFT, 2, y);
    return l;
}

lv_obj_t *zmk_display_status_screen(void) {
    lv_obj_t *screen = lv_obj_create(NULL);

    lbl_title = mk_label(screen, &lv_font_montserrat_12, 0);
    lv_label_set_text(lbl_title, "LP63  ESB 1M");
    lbl_link = mk_label(screen, &lv_font_montserrat_16, 15);
    lbl_radio = mk_label(screen, &lv_font_montserrat_12, 34);
    lbl_batt = mk_label(screen, &lv_font_montserrat_16, 49);

    refresh(NULL);
    lv_timer_create(refresh, 1000, NULL);
    return screen;
}
