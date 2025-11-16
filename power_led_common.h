#pragma once
#include <linux/device.h>

struct syno_wrapper {
    struct device *dev;
    void *phy;
    const struct wrapper_phy_ops *phy_ops;

    struct workqueue_struct *wq;
    struct delayed_work periodic_work;
};

struct wrapper_phy_ops {
    int (*send_cmd)(struct syno_wrapper *, const u8 *);
};

struct syno_wrapper *power_led_common_init(void *phy,
    const struct wrapper_phy_ops *phy_ops,
    struct device *dev);
void power_led_common_cleanup(struct syno_wrapper *priv);