#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/gpio/consumer.h>

#include "arch/apollolake_common.h"

MODULE_LICENSE("GPL");

struct gpio_desc *power;

int init_module(void)
{
	printk(KERN_INFO "Hello world 1.\n");
	gpiod_add_lookup_table(&apollolake_gpios_table);

	power = gpiod_get(NULL, "power-led", GPIOD_OUT_LOW);

	/* 
	 * A non 0 return means init_module failed; module can't be loaded. 
	 */
	return 0;
}

void cleanup_module(void)
{
	gpiod_set_value_cansleep(power, 1);
	gpiod_remove_lookup_table(&apollolake_gpios_table);
	printk(KERN_INFO "Goodbye world 1.\n");
}