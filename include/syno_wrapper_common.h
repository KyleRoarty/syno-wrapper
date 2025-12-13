#pragma once
#include <linux/device.h>
#include <linux/miscdevice.h>

struct syno_wrapper {
    struct device *dev;
    void *phy;
    const struct wrapper_phy_ops *phy_ops;

    struct workqueue_struct *wq;
    struct miscdevice wrapper_misc;
};

struct wrapper_phy_ops {
    int (*send_cmd)(struct syno_wrapper *, const u8 *);
};

struct syno_wrapper *syno_wrapper_common_init(void *phy,
    const struct wrapper_phy_ops *phy_ops,
    struct device *dev);
void syno_wrapper_common_cleanup(struct syno_wrapper *priv);