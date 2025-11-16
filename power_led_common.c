#include <linux/module.h> /* Needed by all modules */
#include <linux/kernel.h> /* Needed for KERN_INFO */
#include <linux/gpio/consumer.h>

#include "arch/apollolake_common.h"
#include "power_led_common.h"

#define INTERVAL 1000

static void light_state_toggle(struct work_struct *work) {
	printk(KERN_INFO "In light state toggle\n");
	static int light_state = 0;
	struct syno_wrapper *dev = container_of(work, struct syno_wrapper,
		periodic_work.work);

	u8 *cmd = NULL; 

	switch (light_state)
	{
	case 0:
		cmd = "4";
		break;
	case 1:
		cmd = "5";
		break;
	case 2:
		cmd = "6";
		break;
	default:
		printk(KERN_INFO "Somehow outside the state bounds\n");
	 	return;
	}

	printk(KERN_INFO "cmd: %s\n", cmd);

	//light_state = (light_state + 1) % 3;

	int ret = dev->phy_ops->send_cmd(dev, cmd);

	printk(KERN_INFO "Return value: %d\n", ret);

	//queue_delayed_work(dev->wq, &dev->periodic_work,
	//	msecs_to_jiffies(INTERVAL));
}

struct syno_wrapper *power_led_common_init(void *phy,
	const struct wrapper_phy_ops *phy_ops,
	struct device *dev)
{
	printk(KERN_INFO "Hello world 1.\n");
	struct syno_wrapper *priv;
	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (priv == NULL) {
		return ERR_PTR(-ENOMEM);
	}
	priv->dev = dev;
	priv->phy = phy;
	priv->phy_ops = phy_ops;

	INIT_DELAYED_WORK(&priv->periodic_work, light_state_toggle);
	priv->wq = alloc_ordered_workqueue("syno_wrapper", 0);
	if (priv->wq == NULL) {
		return ERR_PTR(-ENOMEM);
	}

	queue_delayed_work(priv->wq, &priv->periodic_work,
		msecs_to_jiffies(INTERVAL));

	gpiod_add_lookup_table(&apollolake_gpios_table);

	return priv;

	//power = gpiod_get(NULL, "power-led", GPIOD_OUT_LOW);
	//gpiod_set_value_cansleep(power, 0);

	return priv;
}
EXPORT_SYMBOL_GPL(power_led_common_init);

void power_led_common_cleanup(struct syno_wrapper *priv)
{
	cancel_delayed_work_sync(&priv->periodic_work);
	destroy_workqueue(priv->wq);
	//gpiod_set_value_cansleep(power, 1);
	//gpiod_put(power);
	gpiod_remove_lookup_table(&apollolake_gpios_table);
	kfree(priv);
	printk(KERN_INFO "Goodbye world 1.\n");
}
EXPORT_SYMBOL_GPL(power_led_common_cleanup);

MODULE_LICENSE("GPL");