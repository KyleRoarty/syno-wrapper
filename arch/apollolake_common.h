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
#define SZ_UART_CMD_PREFIX         "-"
#define SZ_UART_ALARM_LED_ON       "LA1"
#define SZ_UART_ALARM_LED_BLINKING "LA2"
#define SZ_UART_ALARM_LED_OFF      "LA3"
#define SZ_UART_FAN_DUTY_CYCLE     "V"
#define SZ_UART_FAN_FREQUENCY      "W"
#define SZ_UART_CPUFAN_DUTY_CYCLE  "X"
#define SZ_UART_CPUFAN_FREQUENCY   "Y"
#define SZ_UART_PWR_LED_ON         "4"
#define SZ_UART_PWR_LED_OFF        "6"

#include <linux/gpio/machine.h>

struct gpiod_lookup_table apollolake_gpios_table = {
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
