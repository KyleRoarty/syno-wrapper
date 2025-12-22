#include <asm-generic/errno-base.h>
#include <linux/container_of.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/gfp_types.h>
#include <linux/gpio/consumer.h>
#include <linux/device/devres.h>
#include <linux/device.h>
#include <linux/bitmap.h>
#include <linux/bitops.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/workqueue.h>
#include <linux/types.h>
#include <linux/workqueue_types.h>

#include "include/backplanectrl.h"

#define BP_POLL_MS 2000

struct bp_ctrl {
	unsigned int nslots;
	struct delayed_work work;
	struct gpio_descs *sense_pins;
	struct gpio_descs *power_pins;
	unsigned long *prev_sense;
};

#define BP_MAX_SLOTS 32
static void backplanectrl_work_fn(struct work_struct *work)
{
	DECLARE_BITMAP(curr_sense, BP_MAX_SLOTS);
	DECLARE_BITMAP(changed, BP_MAX_SLOTS);

	struct bp_ctrl *bp =
		container_of(to_delayed_work(work), struct bp_ctrl, work);

	// Could maybe segment this all out if I want to
	// run this at creation time?
	gpiod_get_array_value_cansleep(bp->nslots, bp->sense_pins->desc,
				       bp->sense_pins->info, curr_sense);

	bitmap_xor(changed, curr_sense, bp->prev_sense, bp->nslots);
	if (!bitmap_empty(changed, bp->nslots)) {
		unsigned int i;

		for_each_set_bit(i, changed, bp->nslots) {
			gpiod_set_value_cansleep(bp->power_pins->desc[i],
						 test_bit(i, curr_sense));
			pr_info("Set slot %d to %d\n", i,
				test_bit(i, curr_sense));
		}

		bitmap_copy(bp->prev_sense, curr_sense, bp->nslots);
	}

	schedule_delayed_work(&bp->work, msecs_to_jiffies(BP_POLL_MS));
	return;
}

void backplanectrl_start(struct bp_ctrl *bp)
{
	if (!bp)
		return;
	schedule_delayed_work(&bp->work, 0);
}

struct bp_ctrl *backplanectrl_create(struct device *dev, const char *sense_str,
				     const char *power_str)
{
	struct gpio_descs *sense_array;
	struct gpio_descs *power_array;
	struct bp_ctrl *bp;

	sense_array = devm_gpiod_get_array(dev, sense_str, GPIOD_IN);
	if (IS_ERR(sense_array))
		return ERR_CAST(sense_array);

	power_array = devm_gpiod_get_array(dev, power_str, GPIOD_ASIS);
	if (IS_ERR(power_array))
		return ERR_CAST(power_array);

	if (sense_array->ndescs != power_array->ndescs) {
		pr_err("Backplane has %u sense pins and %u power pins!",
		       sense_array->ndescs, power_array->ndescs);
		return ERR_PTR(-EINVAL);
	}

	bp = devm_kzalloc(dev, sizeof(*bp), GFP_KERNEL);
	if (!bp)
		return ERR_PTR(-ENOMEM);

	bp->sense_pins = sense_array;
	bp->power_pins = power_array;
	bp->nslots = sense_array->ndescs;
	bp->prev_sense = devm_bitmap_zalloc(dev, bp->nslots, GFP_KERNEL);
	if (!bp->prev_sense)
		return ERR_PTR(-ENOMEM);

	for (int i = 0; i < bp->nslots; i++) {
		int ret = gpiod_set_debounce(sense_array->desc[i], 5000);
		if (ret == -ENOTSUPP)
			continue;
		if (ret)
			return ERR_PTR(ret);
	}

	gpiod_get_array_value_cansleep(bp->nslots, bp->sense_pins->desc,
				       bp->sense_pins->info, bp->prev_sense);

	for (int i = 0; i < bp->nslots; i++) {
		int ret = gpiod_direction_output(bp->power_pins->desc[i],
						 test_bit(i, bp->prev_sense));
		if (ret)
			return ERR_PTR(ret);

		pr_info("Initialized slot %d to %d\n", i,
			test_bit(i, bp->prev_sense));
	}

	INIT_DELAYED_WORK(&bp->work, backplanectrl_work_fn);

	return bp;
}

void backplanectrl_cleanup(struct bp_ctrl *bp)
{
	if (!bp)
		return;
	cancel_delayed_work_sync(&bp->work);
}

MODULE_SOFTDEP("post: ahci");