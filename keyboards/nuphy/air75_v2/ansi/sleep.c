/*
Copyright 2023 @ Nuphy <https://nuphy.com/>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "user_kb.h"
#include "ansi.h"
#include "hal_usb.h"
#include "usb_main.h"
#include "mcu_pwr.h"
#include "rf_queue.h"

extern bool            f_goto_sleep;
extern bool            f_goto_deepsleep;

void set_side_rgb(uint8_t side, uint8_t r, uint8_t g, uint8_t b);
void side_rgb_refresh(void);

void signal_sleep(uint8_t r, uint8_t g, uint8_t b) {
    // Visual cue for sleep on side LED.
    pwr_side_led_on();
    wait_ms(50); // give some time to ensure LED powers on.
    set_side_rgb(3, r, g, b);
    side_rgb_refresh();
    wait_ms(300);
}

void deep_sleep_handle(void) {
    break_all_key(); // reset keys before sleeping for new QMK lifecycle to handle on wake.

    signal_sleep(0x00, 0x80, 0x00);

    // Sync again before sleeping
    dev_sts_sync();
    enter_deep_sleep(); // puts the board in WFI mode and pauses the MCU
#if (MCU_SLEEP_ENABLE)
    exit_deep_sleep();  // This gets called when there is an interrupt (wake) event.
    no_act_time = 0; // required to not cause an immediate sleep on first wake
#endif
}

/**
 * @brief  Sleep Handle.
 */
void sleep_handle(void) {
    static uint32_t delay_step_timer = 0;
    static uint8_t  usb_suspend_debounce = 0;
    static uint32_t rf_disconnect_time = 0;

    /* 50ms interval */
    if (timer_elapsed32(delay_step_timer) < 50) return;
        delay_step_timer = timer_read32();

    if (user_config.sleep_enable % 2 != 1 || f_rf_sleep)
        f_goto_deepsleep = 0;
    else if (no_act_time >= deep_sleep_delay && dev_info.link_mode == LINK_RF_24)
        f_goto_deepsleep = 1;
    else if (no_act_time >= (5 * deep_sleep_delay))
        f_goto_deepsleep = 1;


    if (f_goto_deepsleep != 0) {
        f_goto_deepsleep = 0;
        f_goto_sleep     = 0;
        f_wakeup_prepare = 1;
        f_rf_sleep       = 1;
        deep_sleep_handle();
        return;
    }

    // sleep process
    if (f_goto_sleep) {
        f_goto_sleep       = 0;
        rf_disconnect_time = 0;
        rf_linking_time    = 0;

        if(user_config.sleep_enable != 0) {
            // Visual cue for sleep on side LED.
            if (user_config.sleep_enable == 2) signal_sleep(0x00, 0x00, 0x80);
            enter_light_sleep();
        }

        f_wakeup_prepare = 1;
    }

    // wakeup check
    if (f_wakeup_prepare) {
        if (no_act_time <= 5) { // activity wake up
            f_wakeup_prepare = 0;
            f_rf_sleep       = 0;
            if (user_config.sleep_enable != 0) exit_light_sleep(false);
        }
    }

    // sleep check
    if (f_goto_sleep || f_wakeup_prepare)
        return;
    if (dev_info.link_mode == LINK_USB) {
        if (USB_DRIVER.state == USB_SUSPENDED) {
            usb_suspend_debounce++;
            if (usb_suspend_debounce >= 20) {
                f_goto_sleep = 1;
            }
        } else {
            usb_suspend_debounce = 0;
        }
    } else if (no_act_time >= sleep_time_delay && user_config.sleep_enable != 0) {
        f_goto_sleep     = 1;
    } else if (rf_linking_time >= (dev_info.link_mode == LINK_RF_24 ? (link_timeout / 4) : link_timeout)) {
        f_goto_deepsleep    = 1;
        f_goto_sleep        = 1;
    } else if (dev_info.rf_state == RF_DISCONNECT) {
            rf_disconnect_time++;
        if (rf_disconnect_time > 5 * 20) {
            f_goto_deepsleep = 1;
            f_goto_sleep     = 1;
        }
    } else if (dev_info.rf_state == RF_CONNECT) {
            rf_disconnect_time = 0;
    }
}
