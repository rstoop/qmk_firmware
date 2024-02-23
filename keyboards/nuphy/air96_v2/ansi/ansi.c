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
#include "usb_main.h"
#include "mcu_pwr.h"

extern bool            f_rf_sw_press;
extern bool            f_sleep_show;
extern bool            f_dev_reset_press;
extern bool            f_bat_num_show;
extern bool            f_rgb_test_press;
extern bool            f_bat_hold;
extern uint32_t        no_act_time;
extern uint8_t         rf_sw_temp;
extern uint16_t        rf_sw_press_delay;
extern uint16_t        rf_linking_time;
extern uint16_t        sleep_time_delay;
extern user_config_t   user_config;
extern DEV_INFO_STRUCT dev_info;
extern uint16_t        link_timeout;
extern uint16_t        link_timeout_rf24;
extern uint32_t        deep_sleep_delay;
extern uint32_t        uart_rpt_timer;
extern uint32_t        eeprom_update_timer;
extern bool            rgb_update;

/* qmk process record */
bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    no_act_time       = 0;
    rf_linking_time   = 0;
    uart_rpt_timer    = timer_read32();  // reset uart repeat timer.
    static uint16_t   numlock_timer;

    if (!process_record_user(keycode, record)) {
        return false;
    }

    switch (keycode) {
        case RF_DFU:
            if (record->event.pressed) {
                if (dev_info.link_mode != LINK_USB) return false;
                uart_send_cmd(CMD_RF_DFU, 10, 20);
            }
            return false;

        case LNK_USB:
            if (record->event.pressed) {
                break_all_key();
            } else {
                dev_info.link_mode = LINK_USB;
                uart_send_cmd(CMD_SET_LINK, 10, 10);
            }
            return false;

        case LNK_RF:
            if (record->event.pressed) {
                if (dev_info.link_mode != LINK_USB) {
                    rf_sw_temp    = LINK_RF_24;
                    f_rf_sw_press = 1;
                    break_all_key();
                }
            } else if (f_rf_sw_press) {
                f_rf_sw_press = 0;

                if (rf_sw_press_delay < RF_LONG_PRESS_DELAY) {
                    dev_info.link_mode   = rf_sw_temp;
                    dev_info.rf_channel  = rf_sw_temp;
                    dev_info.ble_channel = rf_sw_temp;
                    uart_send_cmd(CMD_SET_LINK, 10, 20);
                }
            }
            return false;

        case LNK_BLE1:
            if (record->event.pressed) {
                if (dev_info.link_mode != LINK_USB) {
                    rf_sw_temp    = LINK_BT_1;
                    f_rf_sw_press = 1;
                    break_all_key();
                }
            } else if (f_rf_sw_press) {
                f_rf_sw_press = 0;

                if (rf_sw_press_delay < RF_LONG_PRESS_DELAY) {
                    dev_info.link_mode   = rf_sw_temp;
                    dev_info.rf_channel  = rf_sw_temp;
                    dev_info.ble_channel = rf_sw_temp;
                    uart_send_cmd(CMD_SET_LINK, 10, 20);
                }
            }
            return false;

        case LNK_BLE2:
            if (record->event.pressed) {
                if (dev_info.link_mode != LINK_USB) {
                    rf_sw_temp    = LINK_BT_2;
                    f_rf_sw_press = 1;
                    break_all_key();
                }
            } else if (f_rf_sw_press) {
                f_rf_sw_press = 0;

                if (rf_sw_press_delay < RF_LONG_PRESS_DELAY) {
                    dev_info.link_mode   = rf_sw_temp;
                    dev_info.rf_channel  = rf_sw_temp;
                    dev_info.ble_channel = rf_sw_temp;
                    uart_send_cmd(CMD_SET_LINK, 10, 20);
                }
            }
            return false;

        case LNK_BLE3:
            if (record->event.pressed) {
                if (dev_info.link_mode != LINK_USB) {
                    rf_sw_temp    = LINK_BT_3;
                    f_rf_sw_press = 1;
                    break_all_key();
                }
            } else if (f_rf_sw_press) {
                f_rf_sw_press = 0;

                if (rf_sw_press_delay < RF_LONG_PRESS_DELAY) {
                    dev_info.link_mode   = rf_sw_temp;
                    dev_info.rf_channel  = rf_sw_temp;
                    dev_info.ble_channel = rf_sw_temp;
                    uart_send_cmd(CMD_SET_LINK, 10, 20);
                }
            }
            return false;

        case MAC_VOICE:
            if (record->event.pressed) {
                if (dev_info.sys_sw_state == SYS_SW_MAC) {
                    host_consumer_send(0xcf);
                } else {
                    register_code(KC_F5);
                    wait_ms(10);
                    unregister_code(KC_F5);
                }
            } else if (dev_info.sys_sw_state == SYS_SW_MAC) {
                host_consumer_send(0);
            }
            return false;

        case MAC_DND:
            if (record->event.pressed) {
                host_system_send(0x9b);
            } else {
                host_system_send(0);
            }
            return false;

        case TASK:
            if (record->event.pressed) {
                if (dev_info.sys_sw_state == SYS_SW_MAC) {
                    register_code(KC_MCTL);
                    wait_ms(10);
                    unregister_code(KC_MCTL);
                } else {
                    register_code(KC_CALC);
                    wait_ms(10);
                    unregister_code(KC_CALC);
                }
            }
            return false;

        case SEARCH:
            if (record->event.pressed) {
                if (dev_info.sys_sw_state == SYS_SW_MAC) {
                    register_code(KC_LGUI);
                    register_code(KC_SPACE);
                    wait_ms(10);
                    unregister_code(KC_LGUI);
                    unregister_code(KC_SPACE);
                } else {
                    register_code(KC_LCTL);
                    register_code(KC_F);
                    wait_ms(10);
                    unregister_code(KC_F);
                    unregister_code(KC_LCTL);
                }
            }
            return false;

        case PRT_SCR:
            if (record->event.pressed) {
                if (dev_info.sys_sw_state == SYS_SW_MAC) {
                    register_code(KC_LGUI);
                    register_code(KC_LSFT);
                    register_code(KC_3);
                    wait_ms(10);
                    unregister_code(KC_3);
                    unregister_code(KC_LSFT);
                    unregister_code(KC_LGUI);
                } else {
                    register_code(KC_PSCR);
                    wait_ms(10);
                    unregister_code(KC_PSCR);
                }
            }
            return false;

        case PRT_AREA:
            if (record->event.pressed) {
                if (dev_info.sys_sw_state == SYS_SW_MAC) {
                    register_code(KC_LGUI);
                    register_code(KC_LSFT);
                    register_code(KC_4);
                    wait_ms(10);
                    unregister_code(KC_4);
                    unregister_code(KC_LSFT);
                    unregister_code(KC_LGUI);
                }
                else {
                    register_code(KC_PSCR);
                    wait_ms(10);
                    unregister_code(KC_PSCR);
                }
            }
            return false;

        case SIDE_VAI:
            if (record->event.pressed) {
                side_light_control(1);
            }
            return false;

        case SIDE_VAD:
            if (record->event.pressed) {
                side_light_control(0);
            }
            return false;

        case SIDE_MOD:
            if (record->event.pressed) {
                side_mode_control(1);
            }
            return false;

        case SIDE_HUI:
            if (record->event.pressed) {
                side_colour_control(1);
            }
            return false;

        case SIDE_SPI:
            if (record->event.pressed) {
                side_speed_control(1);
            }
            return false;

        case SIDE_SPD:
            if (record->event.pressed) {
                side_speed_control(0);
            }
            return false;

        case SIDE_1:
            if (record->event.pressed) {
                side_one_control(1);
            }
            return false;

        case RGB_VAI:
            if (record->event.pressed) {
                rgb_matrix_increase_val_noeeprom();
                eeprom_update_timer = timer_read32();
                rgb_update = 1;
            }
            return false;

        case RGB_VAD:
            if (record->event.pressed) {
                rgb_matrix_decrease_val_noeeprom();
                eeprom_update_timer = timer_read32();
                rgb_update = 1;
            }
            return false;

        case RGB_MOD:
            if (record->event.pressed) {
                rgb_matrix_step_noeeprom();
                eeprom_update_timer = timer_read32();
                rgb_update = 1;
            }
            return false;

        case RGB_HUI:
            if (record->event.pressed) {
                rgb_matrix_increase_hue_noeeprom();
                eeprom_update_timer = timer_read32();
                rgb_update = 1;
            }
            return false;

        case RGB_SPI:
            if (record->event.pressed) {
                rgb_matrix_increase_speed_noeeprom();
                eeprom_update_timer = timer_read32();
                rgb_update = 1;
            }
            return false;

        case RGB_SPD:
            if (record->event.pressed) {
                rgb_matrix_decrease_speed_noeeprom();
                eeprom_update_timer = timer_read32();
                rgb_update = 1;
            }
            return false;

        case DEV_RESET:
            if (record->event.pressed) {
                f_dev_reset_press = 1;
                break_all_key();
            } else {
                f_dev_reset_press = 0;
            }
            return false;

        case SLEEP_MODE:
            if (record->event.pressed) {
                if(user_config.sleep_enable) {
                    if (sleep_time_delay < (100 * 360) ) {
                        link_timeout = (100 * 120);
                        link_timeout_rf24 = (100 * 120);
                        sleep_time_delay = (100 * 360);
                    } else {
                        user_config.sleep_enable = false;
                    }
                } else {
                    link_timeout = (50 + 100 * 59);
                    link_timeout_rf24 = (100 * 10);
                    sleep_time_delay = (100 * 60 * 2);
                    user_config.sleep_enable = true;
                }
                f_sleep_show       = 1;
                eeconfig_update_kb_datablock(&user_config);
            }
            return false;

        case BAT_SHOW:
            if (record->event.pressed) {
                f_bat_hold = !f_bat_hold;
            }
            return false;

        case BAT_NUM:
            f_bat_num_show = record->event.pressed;
            return false;

        case RGB_TEST:
            f_rgb_test_press = record->event.pressed;
            return false;

        case NUMLOCK_INS:
            if (record->event.pressed) {
                numlock_timer = timer_read();
                if (get_mods() & MOD_MASK_CSA) {
                    numlock_timer = 0;
                    register_code(KC_INS);
                    wait_ms(10);
                    unregister_code(KC_INS);
                }
            } else {
                if (numlock_timer == 0) {
                } else if (timer_elapsed(numlock_timer) > TAPPING_TERM) {
                    numlock_timer = 0;
                    register_code(KC_NUM);
                    wait_ms(10);
                    unregister_code(KC_NUM);
                } else {
                    numlock_timer = 0;
                    register_code(KC_INS);
                    wait_ms(10);
                    unregister_code(KC_INS);
                }
            }
            return false;
    }
    return true;
}

bool rgb_matrix_indicators_kb(void) {
    if (!rgb_matrix_indicators_user()) {
        return false;
    }

    if(f_bat_num_show) {
        num_led_show();
    }

    // power down unused LEDs
    led_power_handle();
    return true;
}

/* qmk keyboard post init */
void keyboard_post_init_kb(void) {
    gpio_init();
    rf_uart_init();
    wait_ms(500);
    rf_device_init();

    break_all_key();
    dial_sw_fast_scan();
    load_eeprom_data();
    keyboard_post_init_user();
}

/* qmk housekeeping task */
void housekeeping_task_kb(void) {
    timer_pro();

    uart_receive_pro();

    uart_send_report_repeat();

    dev_sts_sync();

    long_press_key();

    dial_sw_scan();

    side_led_show();

    delay_update_eeprom_data();

    sleep_handle();
}
