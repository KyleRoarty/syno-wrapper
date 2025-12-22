#ifndef _SYNO_WRAPPER_COMMON_H_
#define _SYNO_WRAPPER_COMMON_H_
#include <asm-generic/int-ll64.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include "include/fanctl.h"
#include "include/backplanectrl.h"

struct syno_wrapper {
    struct device *dev;
    void *phy;
    struct fan_ctrl *fc;
    struct bp_ctrl *bp;
    const struct wrapper_phy_ops *phy_ops;
    struct workqueue_struct *wq;
    struct miscdevice wrapper_misc;
};

struct wrapper_phy_ops {
    int (*send_cmd)(void *, const u8 *);
};

struct syno_wrapper *syno_wrapper_common_init(void *phy,
    const struct wrapper_phy_ops *phy_ops,
    struct device *dev);
void syno_wrapper_common_cleanup(struct syno_wrapper *priv);
#endif // _SYNO_WRAPPER_COMMON_H_
