/* 
Copyright 2023 NuPhy, Persama (@Persama) & jincao1
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
#include "mcu_stm32f0xx.h"
#include "mcu_pwr.h"
#include "i2c_master.h"


//------------------------------------------------
extern DEV_INFO_STRUCT dev_info;

static bool f_usb_deinit         = 0;
static bool rgb_led_powered_off  = 0;
static bool tim6_enabled         = false;
static bool rgb_led_on  = 0;

void clear_report_buffer(void);

#if (MCU_SLEEP_ENABLE)

// Pin definitions
static const pin_t row_pins[MATRIX_ROWS] = MATRIX_ROW_PINS;
static const pin_t col_pins[MATRIX_COLS] = MATRIX_COL_PINS;


/** ================================================================
 * @brief  Turn off USB
 *
 ================================================================*/
void m_deinit_usb_072(void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};

#if (0)
    // call qmk library to turn off USB
    void close_usb(void);
    close_usb();
#endif

    // Reset USB register
    RCC_APB1PeriphResetCmd(RCC_APB1RSTR_USBRST, ENABLE);
    RCC_APB1PeriphResetCmd(RCC_APB1RSTR_USBRST, DISABLE);
    wait_ms(10);

    // Turn off USB clock
    RCC_APB1PeriphClockCmd(RCC_APB1ENR_USBEN, DISABLE);

    // GPIO to suspended state
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init((GPIO_TypeDef *)GPIOA, &GPIO_InitStructure);
}

/** ================================================================
 * @brief   Low Power mode
 *
 ================================================================*/
#include "hal_usb.h"
#include "usb_main.h"
void SYSCFG_EXTILineConfig(uint8_t EXTI_PortSourceGPIOx, uint8_t EXTI_PinSourcex) {
    uint32_t tmp = 0x00;

    tmp = ((uint32_t)0x0F) << (0x04 * (EXTI_PinSourcex & (uint8_t)0x03));
    SYSCFG->EXTICR[EXTI_PinSourcex >> 0x02] &= ~tmp;
    SYSCFG->EXTICR[EXTI_PinSourcex >> 0x02] |= (((uint32_t)EXTI_PortSourceGPIOx) << (0x04 * (EXTI_PinSourcex & (uint8_t)0x03)));
}

#include "hal_lld.h"
#define EXTI_PortSourceGPIOA ((uint8_t)0x00)
#define EXTI_PortSourceGPIOB ((uint8_t)0x01)
#define EXTI_PortSourceGPIOC ((uint8_t)0x02)
#define EXTI_PortSourceGPIOD ((uint8_t)0x03)
#endif

#include "usb_main.h"
/**
 * @brief  Enter deep sleep
 * @note This is Nuphy's un-released logic with some cleanup/refactoring
 *       The MCU is put on a low power mode.
 */
void enter_deep_sleep(void) {
    //------------------------ RF to sleep
    uart_send_cmd(CMD_SLEEP, 5, 5);

    led_pwr_sleep_handle();
    clear_report_buffer();

#if (MCU_SLEEP_ENABLE)
    //------------------------ Turn off USB if not used
    if (dev_info.link_mode != LINK_USB) {
        f_usb_deinit = 1;
        m_deinit_usb_072();
    }

    // Close timer
    if (tim6_enabled) TIM_Cmd(TIM6, DISABLE);

    //------------------------ Configure WakeUp Key
/*
    setPinOutput(KCOL_0);
    writePinHigh(KCOL_0);
    setPinOutput(KCOL_1);
    writePinHigh(KCOL_1);
    setPinOutput(KCOL_2);
    writePinHigh(KCOL_2);
    setPinOutput(KCOL_3);
    writePinHigh(KCOL_3);
    setPinOutput(KCOL_4);
    writePinHigh(KCOL_4);
    setPinOutput(KCOL_5);
    writePinHigh(KCOL_5);
    setPinOutput(KCOL_6);
    writePinHigh(KCOL_6);
    setPinOutput(KCOL_7);
    writePinHigh(KCOL_7);
    setPinOutput(KCOL_8);
    writePinHigh(KCOL_8);
    setPinOutput(KCOL_9);
    writePinHigh(KCOL_9);
    setPinOutput(KCOL_10);
    writePinHigh(KCOL_10);
    setPinOutput(KCOL_11);
    writePinHigh(KCOL_11);
    setPinOutput(KCOL_12);
    writePinHigh(KCOL_12);
    setPinOutput(KCOL_13);
    writePinHigh(KCOL_13);
    setPinOutput(KCOL_14);
    writePinHigh(KCOL_14);
    setPinOutput(KCOL_15);
    writePinHigh(KCOL_15);
    setPinOutput(KCOL_16);
    writePinHigh(KCOL_16);
    setPinOutput(KCOL_17);
    writePinHigh(KCOL_17);
    setPinOutput(KCOL_18);
    writePinHigh(KCOL_18);
    setPinOutput(KCOL_19);
    writePinHigh(KCOL_19);
    setPinOutput(KCOL_20);
    writePinHigh(KCOL_20);
*/

/*
    setPinInputLow(KROW_0);
    setPinInputLow(KROW_1);
    setPinInputLow(KROW_2);
    setPinInputLow(KROW_3);
    setPinInputLow(KROW_4);
    setPinInputLow(KROW_5);
*/

    for (int i = 0; i < ARRAY_SIZE(col_pins); ++i) {
        setPinOutput(col_pins[i]);
        writePinHigh(col_pins[i]);
    }
    for (int i = 0; i < ARRAY_SIZE(row_pins); ++i) {
        setPinInputLow(row_pins[i]);
    }

    // Configure interrupt source - all 5 rows of the keyboard.
    SYSCFG_EXTILineConfig(EXTI_PORT_R0, EXTI_PIN_R0);
    SYSCFG_EXTILineConfig(EXTI_PORT_R1, EXTI_PIN_R1);
    SYSCFG_EXTILineConfig(EXTI_PORT_R2, EXTI_PIN_R2);
    SYSCFG_EXTILineConfig(EXTI_PORT_R3, EXTI_PIN_R3);
    SYSCFG_EXTILineConfig(EXTI_PORT_R4, EXTI_PIN_R4);
    SYSCFG_EXTILineConfig(EXTI_PORT_R5, EXTI_PIN_R5);

    EXTI_InitTypeDef m_exti;
    EXTI_StructInit(&m_exti);
    m_exti.EXTI_Line    = 0XFFFF; // GPIO 0-15
    m_exti.EXTI_LineCmd = ENABLE;
    m_exti.EXTI_Mode    = EXTI_Mode_Interrupt;
    m_exti.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&m_exti);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel         = EXTI4_15_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd      = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_1_IRQn;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_3_IRQn;
    NVIC_Init(&NVIC_InitStructure);


    // led_pwr_sleep_handle();

    setPinOutput(DEV_MODE_PIN);
    writePinLow(DEV_MODE_PIN);

    setPinOutput(SYS_MODE_PIN);
    writePinLow(SYS_MODE_PIN);

    setPinOutput(A11);
    writePinLow(A11);
    setPinOutput(A12);
    writePinLow(A12);

    setPinInputHigh(NRF_BOOT_PIN);

    setPinOutput(NRF_WAKEUP_PIN);
    writePinHigh(NRF_WAKEUP_PIN);

    // Enter low power mode and wait for interrupt signal
    // PWR_EnterSTOPMode(PWR_Regulator_ON, PWR_STOPEntry_WFI);
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
    // PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFE);
#endif
}

/**
 * @brief Wake up from deep sleep
 * @note This is triggered by an interrupt event.
 *       This is mostly Nuphy's unreleased logic with cleanup/refactoring by me.
 */
void exit_deep_sleep(void) {
 
    // Matrix initialization
    extern void matrix_init_pins(void);
    matrix_init_pins();

    // Restore IO to working status
    setPinInputHigh(DEV_MODE_PIN); // PC0
    setPinInputHigh(SYS_MODE_PIN); // PC1

    /* set RF module boot pin high */
    // setPinInputHigh(NRF_BOOT_PIN);

    /* Wake RF module */
    setPinOutput(NRF_WAKEUP_PIN);
    // writePinHigh(NRF_WAKEUP_PIN);

    // Power on LEDs This is missing from Nuphy's logic.
    led_pwr_wake_handle();
    rgb_matrix_set_color(RGB_MATRIX_LED_COUNT-1, 0, 0, 0);

    // Reinitialize the system clock
    stm32_clock_init();

    /* TIM6 enable */
    if (tim6_enabled) TIM_Cmd(TIM6, ENABLE);

    // Handshake send to wake RF
    uart_send_cmd(CMD_HAND, 0, 1);

    // Should re-init USB regardless probably if it was deinitialized.
    if (f_usb_deinit) {
        usbWakeupHost(&USB_DRIVER);
        restart_usb_driver(&USB_DRIVER);
        f_usb_deinit = 0;
    }
    // flag for RF wakeup workload.
    dev_info.rf_state = RF_WAKE;
}

/**
 * @brief  Light sleep by powering off LEDs.
 * @note This is Nuphy's "open sourced" sleep logic. It's not deep sleep.
 */
void enter_light_sleep(void) {
    if (dev_info.rf_state == RF_CONNECT)
        uart_send_cmd(CMD_SET_CONFIG, 5, 5);
    else
        uart_send_cmd(CMD_SLEEP, 5, 5);

    led_pwr_sleep_handle();
    clear_report_buffer();
}

/**
 * @brief  Power back up LEDs on exiting light sleep.
 * @note This is Nuphy's "open sourced" wake logic. It's not deep sleep.
 */
void exit_light_sleep(void) {
    led_pwr_wake_handle();
    rgb_matrix_set_color(RGB_MATRIX_LED_COUNT-1, 0, 0, 0);
    uart_send_cmd(CMD_HAND, 0, 1);

    if (dev_info.link_mode == LINK_USB) {
        usb_lld_wakeup_host(&USB_DRIVER);
        restart_usb_driver(&USB_DRIVER);
    }

    // flag for RF wakeup workload.
    dev_info.rf_state = RF_WAKE;
}

void led_pwr_sleep_handle(void) {
    // reset the flags.
    rgb_led_powered_off  = 0;

    // power off leds if they were enabled
    if (is_rgb_led_on()) {
        rgb_led_powered_off = 1;
        pwr_led_off();
    }
}

void led_pwr_wake_handle(void) {
    if (rgb_led_powered_off) {
        pwr_led_on();
    }
}

void pwr_led_off(void) {
    if (!rgb_led_on) return;
    // LED power supply off
    setPinOutput(DC_BOOST_PIN);
    writePinLow(DC_BOOST_PIN);
    setPinInput(RGB_DRIVER_SDB1);
    setPinInput(RGB_DRIVER_SDB2);
    rgb_led_on = 0;
}

void pwr_led_on(void) {
    if (rgb_led_on) return;
    // LED power supply on
    setPinOutput(DC_BOOST_PIN);
    writePinHigh(DC_BOOST_PIN);
    setPinOutput(RGB_DRIVER_SDB1);
    writePinHigh(RGB_DRIVER_SDB1);
    setPinOutput(RGB_DRIVER_SDB2);
    writePinHigh(RGB_DRIVER_SDB2);
    rgb_led_on = 1;
}

bool is_rgb_led_on(void) {
    return rgb_led_on;
}

#if (MCU_SLEEP_ENABLE)
/* Nuphy's implementations. */

/**
 * @brief  Clears the EXTI's line pending flags.
 * @param  EXTI_Line: specifies the EXTI lines flags to clear.
 *   This parameter can be any combination of EXTI_Linex where x can be (0..19)
 * @retval None
 */
void EXTI_ClearFlag(uint32_t EXTI_Line) {
    EXTI->PR = EXTI_Line;
}

///////////////////////////////////////////////////////////////////////////////////////////

#define STM32_EXTI_0_1_HANDLER Vector54
OSAL_IRQ_HANDLER(STM32_EXTI_0_1_HANDLER) {
    EXTI->PR = 0xffff;
}

#define STM32_EXTI_2_3_HANDLER Vector58
OSAL_IRQ_HANDLER(STM32_EXTI_2_3_HANDLER) {
    EXTI->PR = 0xffff;
}

#define STM32_EXTI_4_15_HANDLER Vector5C
OSAL_IRQ_HANDLER(STM32_EXTI_4_15_HANDLER) {
    EXTI->PR = 0xffff;
}

void EXTI_DeInit(void) {
    EXTI->IMR  = 0x0F940000;
    EXTI->EMR  = 0x00000000;
    EXTI->RTSR = 0x00000000;
    EXTI->FTSR = 0x00000000;
    EXTI->PR   = 0x006BFFFF;
}

void EXTI_Init(EXTI_InitTypeDef *EXTI_InitStruct) {
    uint32_t tmp = 0;

    tmp = (uint32_t)EXTI_BASE;

    if (EXTI_InitStruct->EXTI_LineCmd != DISABLE) {
        /* Clear EXTI line configuration */
        EXTI->IMR &= ~EXTI_InitStruct->EXTI_Line;
        EXTI->EMR &= ~EXTI_InitStruct->EXTI_Line;

        tmp += EXTI_InitStruct->EXTI_Mode;

        *(__IO uint32_t *)tmp |= EXTI_InitStruct->EXTI_Line;

        /* Clear Rising Falling edge configuration */
        EXTI->RTSR &= ~EXTI_InitStruct->EXTI_Line;
        EXTI->FTSR &= ~EXTI_InitStruct->EXTI_Line;

        /* Select the trigger for the selected interrupts */
        if (EXTI_InitStruct->EXTI_Trigger == EXTI_Trigger_Rising_Falling) {
            /* Rising Falling edge */
            EXTI->RTSR |= EXTI_InitStruct->EXTI_Line;
            EXTI->FTSR |= EXTI_InitStruct->EXTI_Line;
        } else {
            tmp = (uint32_t)EXTI_BASE;
            tmp += EXTI_InitStruct->EXTI_Trigger;

            *(__IO uint32_t *)tmp |= EXTI_InitStruct->EXTI_Line;
        }
    } else {
        tmp += EXTI_InitStruct->EXTI_Mode;

        /* Disable the selected external lines */
        *(__IO uint32_t *)tmp &= ~EXTI_InitStruct->EXTI_Line;
    }
}

/**
 * @brief  Fills each EXTI_InitStruct member with its reset value.
 * @param  EXTI_InitStruct: pointer to a EXTI_InitTypeDef structure which will
 *         be initialized.
 * @retval None
 */
void EXTI_StructInit(EXTI_InitTypeDef *EXTI_InitStruct) {
    EXTI_InitStruct->EXTI_Line    = 0;
    EXTI_InitStruct->EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStruct->EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct->EXTI_LineCmd = DISABLE;
}

#endif

#if (0)
void mcu_timer6_init(void) {
    NVIC_InitTypeDef        NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    /* TIM6 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

    /* TIM6 interrupt design */
    NVIC_InitStructure.NVIC_IRQChannel         = TIM6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd      = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Time Basic timer settings */
    /* This configuration results in 1ms intervals: https://deepbluembedded.com/stm32-timer-interrupt-hal-example-timer-mode-lab/
       Tout = (ARR + 1)(PSC + 1) / Fclk (48mhz for STM32F072)
       0.001 = (1000)(48)/(48_000_000) = 1ms
    */
    TIM_TimeBaseStructure.TIM_Period        = 1000 - 1;
    TIM_TimeBaseStructure.TIM_Prescaler     = 48 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);

    /* TIM6 enable */
    TIM_Cmd(TIM6, ENABLE);

    tim6_enabled = true;
}

/*
 TIM6 handler is triggered by the timer above which is used as an interrupt.
 This is every 1ms so it effectively puts the CPU sleep mode only for 1s.
*/
volatile uint8_t idle_sleep_cnt = 0;
OSAL_IRQ_HANDLER(STM32_TIM6_HANDLER) {
    if (TIM_GetFlagStatus(TIM6, TIM_FLAG_Update) != ST_RESET) {
        TIM_ClearFlag(TIM6, TIM_FLAG_Update);
        idle_sleep_cnt++;
    }
}

// For this to work you need to call m_timer6_init() in ansi.c post kb init user.
// That enables the STM32_TIM6_HANDLER but I think it runs on the interrupt interval of the TIM6 timer.
void idle_enter_sleep(void) {
    TIM6->CNT      = 0;
    idle_sleep_cnt = 0;
    while (idle_sleep_cnt < 1) {
        PWR_EnterSleepMode(PWR_SLEEPEntry_WFI);
    }
}
#endif
