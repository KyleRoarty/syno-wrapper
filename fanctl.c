#include <asm-generic/errno-base.h>
#include <asm-generic/int-ll64.h>
#include <linux/container_of.h>
#include <linux/err.h>
#include <linux/gfp_types.h>
#include <linux/stddef.h>
#include <linux/types.h> // IWYU pragma: keep
#include <linux/slab.h>
#include <linux/thermal.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include <linux/workqueue_types.h>

#include "include/fanctl.h"

// idk 2 seconds I guess
#define FAN_POLL_MS 2000

struct fan_ctrl {
	void *priv;
	const struct fan_ctrl_ops *ops;
	const struct fan_curve *curve;
	struct thermal_zone_device *thermal_zone;
	u8 cur_fan_pct;
	struct delayed_work work;
};

static u8 get_fan_speed(const struct fan_curve *curve, int temp)
{
	if (temp <= curve->points[0].temp)
		return curve->points[0].speed;

	for (size_t i = 0; i < curve->num_points; i++) {
		const struct fan_point p0 = curve->points[i];
		const struct fan_point p1 = curve->points[i + 1];

		if (p0.temp <= temp) {
			int temp_delta = p1.temp - p0.temp;
			int speed_delta = p1.speed - p0.speed;

			// Need to divide last or you'll get truncated oops
			int interp_speed =
				(speed_delta * (temp - p0.temp)) / temp_delta +
				p0.speed;

			// Should I check if it's between 0-99? Probably.
			return (u8)interp_speed;
		}
	}

	return curve->points[curve->num_points - 1].speed;
}

// Can probably chunk the function up into multiple fns but lazy
static void fanctrl_work_fn(struct work_struct *work)
{
	struct fan_ctrl *fc =
		container_of(to_delayed_work(work), struct fan_ctrl, work);

	// Maybe chunk this
	int temp;
	if (thermal_zone_get_temp(fc->thermal_zone, &temp))
		goto requeue;
	// Maybe chunk this end

	u8 fan_pct = get_fan_speed(fc->curve, temp);

	if (fc->cur_fan_pct == fan_pct)
		goto requeue;

	if (!fc->ops->write_fan_speed(fc->priv, fan_pct)) {
		// re-enable maybe later or make debug print
		//pr_info_ratelimited("Fan: Speed=%u @ temp=%d\n", fan_pct, temp);
		fc->cur_fan_pct = fan_pct;
	}

requeue:
	schedule_delayed_work(&fc->work, msecs_to_jiffies(FAN_POLL_MS));
	return;
}

void fanctrl_start(struct fan_ctrl *fc)
{
	if (!fc)
		return;

	schedule_delayed_work(&fc->work, 0);
}

struct fan_ctrl *fanctrl_create(void *priv, const struct fan_ctrl_ops *ops,
				const struct fan_curve *curve,
				const char *thermal_zone)
{
	struct fan_ctrl *fc;

	fc = kzalloc(sizeof(*fc), GFP_KERNEL);
	if (priv == NULL)
		return ERR_PTR(-ENOMEM);

	fc->priv = priv;
	fc->ops = ops;
	fc->curve = curve;
	fc->cur_fan_pct = 0;

	struct thermal_zone_device *tz;
	tz = thermal_zone_get_zone_by_name(thermal_zone);
	if (IS_ERR(tz)) {
		kfree(fc);
		return ERR_CAST(tz);
	}
	fc->thermal_zone = tz;

	INIT_DELAYED_WORK(&fc->work, fanctrl_work_fn);

	return fc;
}

void fanctrl_cleanup(struct fan_ctrl *fc)
{
	if (!fc)
		return;

	cancel_delayed_work_sync(&fc->work);
	kfree(fc);
}
