/*
 * kerf dongle board init.
 * Copyright (c) 2026 Redwan. SPDX-License-Identifier: MIT
 *
 * The nRF52833 is supplied from USB through VDDH, so VDD comes from the internal
 * REG0 regulator. Its factory default output is 1.8 V, which is too low for the
 * blue and green LED dice. Program REG0 to 3.3 V once (UICR is persistent) and
 * reset so the new voltage takes effect.
 */

#include <zephyr/init.h>
#include <zephyr/sys/reboot.h>
#include <hal/nrf_power.h>
#include <nrfx_nvmc.h>

static int kerf_dongle_init(void) {
    if ((nrf_power_mainregstatus_get(NRF_POWER) == NRF_POWER_MAINREGSTATUS_HIGH) &&
        ((NRF_UICR->REGOUT0 & UICR_REGOUT0_VOUT_Msk) ==
         (UICR_REGOUT0_VOUT_DEFAULT << UICR_REGOUT0_VOUT_Pos))) {

        nrfx_nvmc_word_write((uint32_t)&NRF_UICR->REGOUT0,
                             (NRF_UICR->REGOUT0 & ~((uint32_t)UICR_REGOUT0_VOUT_Msk)) |
                                 (UICR_REGOUT0_VOUT_3V3 << UICR_REGOUT0_VOUT_Pos));
        while (!nrfx_nvmc_write_done_check()) {
        }

        /* A reset is required for the new REGOUT0 value to apply */
        NVIC_SystemReset();
    }

    return 0;
}

SYS_INIT(kerf_dongle_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
