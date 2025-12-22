#ifndef _SYNO_WRAPPER_APOLLOLAKE_COMMON_H_
#define _SYNO_WRAPPER_APOLLOLAKE_COMMON_H_
/*
 * Apollo Lake has 4 GPIO controllers with different number of pins which are loosely arranged.
 * We hard-coded the base offsets of these controllers so the GPIO number can be determined at static time,
 * and the ranges are asigned in the pinctrl driver.
 * look up for the pin number in linux-4.4.x/drivers/pinctrl/intel/pinctrl-broxton.c
 * The global gpio number ranges of each controller are listed below -
 * North 0~77;
 * NorthWest 78~154;
 * West 155~201;
 * SouthWest 202~244;
 */

// On DS918+:
// '-' doesn't do anything
// 'L' and 'E' are multi-byte commands that do nothing
// CPUFAN commands (X, Y) don't do anything as there's no CPU fan
// 'M' doesn't do anything. It allegedly toggles which LED is being controlled
// 'R' doesn't do anything. Allegedly "Get unique CMD"

#include <linux/array_size.h>
#include <linux/gpio/machine.h>
#include <linux/stddef.h>

#include "include/fanctl.h"

#define SZ_UART_POWER_OFF                   0x31 // "1"
#define SZ_UART_BUZZER_SHORT                0x32 // "2"
#define SZ_UART_BUZZER_LONG                 0x33 // "3"
#define SZ_UART_PWR_LED_ON                  0x34 // "4"
#define SZ_UART_PWR_LED_BLINK               0x35 // "5"
#define SZ_UART_PWR_LED_OFF                 0x36 // "6"
#define SZ_UART_STATUS_LED_OFF              0x37 // "7"
#define SZ_UART_STATUS_LED_ON               0x38 // "8"
#define SZ_UART_STATUS_LED_BLINK            0x39 // "9"
#define SZ_UART_USB_LED_ON                  0x40
#define SZ_UART_USB_LED_BLINK               0x41
#define SZ_UART_USB_LED_OFF                 0x42

#define SZ_UART_STARTUP                     0x43 // Makes the power LED blink and fans spin up faster like on startup
#define SZ_UART_OUTPUT_STUFF_TOGGLE         0x4f // "O" // Toggles returning data from the UART?

#define SZ_UART_LED_MIRROR_AB               0x54
#define SZ_UART_TOGGLE_FAN_RPS_REPORT       0x55 /* 'U' */
#define SZ_UART_FAN_DUTY_CYCLE              0x56 // "V" + %02d
#define SZ_UART_FAN_FREQUENCY               0x57 // "W" + %02d

#define SZ_UART_BUTTON_USB                  0x60 // '`'
#define SZ_UART_BUTTON_RESET                0x61 // 'a'
// control by up, must echo u first! uP #1
#define SZ_UART_FAN_FAILURE                 0x66 /* 'f' */
// control by up, must echo EC1 first! uP #17
#define SZ_UART_CPUFAN_FAILURE              0x67 /* 'g' */
#define SZ_UART_UNKNOWN                     0x6f // 'o' // Something else with data return toggling
#define SZ_UART_RCPOWEROFF                  0x70 // 'p'
#define SZ_UART_RCPOWERON                   0x71 // 'q'
#define SZ_UART_DISABLE_SCHEDULE_POWERON    0x72 // 'r'
#define SZ_UART_ENABLE_SCHEDULE_POWERON     0x73 // 's'
#define SZ_UART_DISABLE_FANCHECK            0x74 // 't'
#define SZ_UART_ENABLE_FANCHECK             0x75 // 'u'
#define SZ_UART_SOME_MULTIBYTE              0x76 // 'v' 4 character multi-byte command `vNNN`
#define SZ_UART_WOL_ENABLE                  0x6C // "l"
#define SZ_UART_MAX_LENGTH                  4

static struct gpiod_lookup_table apollolake_gpios_table = {
    .dev_id = NULL,
    .table = {
        GPIO_LOOKUP("INT3452:00", 12, "power-led", GPIO_ACTIVE_HIGH),
        GPIO_LOOKUP("INT3452:00", 14, "hdd-led", GPIO_ACTIVE_HIGH),
        GPIO_LOOKUP("INT3452:00", 15, "lan-led", GPIO_ACTIVE_HIGH),
        GPIO_LOOKUP("INT3452:00", 174, "usb-copy-btn", GPIO_ACTIVE_LOW),

        GPIO_LOOKUP_IDX("INT3452:00", 11, "usb-power", 0, GPIO_ACTIVE_HIGH),
        GPIO_LOOKUP_IDX("INT3452:00", 10, "usb-power", 1, GPIO_ACTIVE_HIGH),
        GPIO_LOOKUP_IDX("INT3452:00", 13, "usb-power", 2, GPIO_ACTIVE_HIGH),

        GPIO_LOOKUP_IDX("INT3452:00", 21, "hdd-power", 1, GPIO_ACTIVE_HIGH),
        GPIO_LOOKUP_IDX("INT3452:00", 20, "hdd-power", 2, GPIO_ACTIVE_HIGH),
        GPIO_LOOKUP_IDX("INT3452:00", 19, "hdd-power", 3, GPIO_ACTIVE_HIGH),
        GPIO_LOOKUP_IDX("INT3452:00",  9, "hdd-power", 4, GPIO_ACTIVE_HIGH),

        GPIO_LOOKUP_IDX("INT3452:00", 18, "hdd-detect", 1, GPIO_ACTIVE_LOW),
        GPIO_LOOKUP_IDX("INT3452:02", 24, "hdd-detect", 2, GPIO_ACTIVE_LOW),
        GPIO_LOOKUP_IDX("INT3452:02", 21, "hdd-detect", 3, GPIO_ACTIVE_LOW),
        GPIO_LOOKUP_IDX("INT3452:02", 20, "hdd-detect", 4, GPIO_ACTIVE_LOW),

        GPIO_LOOKUP_IDX("INT3452:00", 17, "fan-sense", 1, GPIO_ACTIVE_LOW),
        GPIO_LOOKUP_IDX("INT3452:00", 16, "fan-sense", 2, GPIO_ACTIVE_LOW),

        { },
    }
};

#if 1
static const struct fan_point ds918_points[] = {
    {40000, 20},
    {65000, 50},
    {80000, 99}
};
#else
static const struct fan_point ds918_points[] = {
    {10000, 20},
    {40000, 99}
};
#endif

static const struct fan_curve ds918_curve = {
    .points = ds918_points,
    .num_points = ARRAY_SIZE(ds918_points),
};

static const char ds918_tz[] = "acpitz";
#endif // _SYNO_WRAPPER_APOLLOLAKE_COMMON_H_
