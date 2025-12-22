#include <asm-generic/errno-base.h>
#include <asm-generic/int-ll64.h>
#include <linux/compiler_types.h>
#include <linux/container_of.h>
#include <linux/err.h>
#include <linux/export.h>
#include <linux/fs.h>
#include <linux/gfp_types.h>
#include <linux/gpio/machine.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/sprintf.h>
#include <linux/stddef.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/workqueue_types.h>

#include "arch/apollolake_common.h"
#include "include/syno_wrapper_common.h"
#include "include/backplanectrl.h"
#include "include/fanctl.h"

struct uart_job {
	struct work_struct work;
	struct syno_wrapper *dev;
	u8 cmdBuf[SZ_UART_MAX_LENGTH + 1];
};

static void send_command(struct work_struct *work)
{
	struct uart_job *job = container_of(work, struct uart_job, work);

	// Maybe do something with return value idk
	job->dev->phy_ops->send_cmd(job->dev->phy, job->cmdBuf);

	kfree(job);
}

static int send_command_wrapper(struct syno_wrapper *priv, const char *buf)
{
	struct uart_job *job;
	job = kmalloc(sizeof(*job), GFP_KERNEL);
	if (!job)
		return -ENOMEM;
	scnprintf(job->cmdBuf, sizeof(job->cmdBuf), "%s", buf);
	job->dev = priv;
	INIT_WORK(&job->work, send_command);
	queue_work(priv->wq, &job->work);

	return 0;
}

static ssize_t wrapper_write(struct file *file, const char __user *buf,
			     size_t count, loff_t *ppos)
{
	struct syno_wrapper *priv = container_of(
		file->private_data, struct syno_wrapper, wrapper_misc);

	if (count > SZ_UART_MAX_LENGTH + 1) {
		pr_info("Too much data - ignoring\n");
		*ppos += count;
		return count;
	}

	char cmdBuf[SZ_UART_MAX_LENGTH + 1];
	if (copy_from_user(cmdBuf, buf, count))
		return -EFAULT;

	pr_info("Received: %s\n", cmdBuf);

	int ret = send_command_wrapper(priv, cmdBuf);
	if (ret)
		return ret;

	*ppos += count;
	return count;
}

static const struct file_operations wrapper_fops = { .owner = THIS_MODULE,
						     .write = wrapper_write };

static int write_fan(void *priv, const u8 fan_pct)
{
	struct syno_wrapper *this = priv;

	if (fan_pct > 99)
		return 1;

	char cmdBuf[SZ_UART_MAX_LENGTH];
	scnprintf(cmdBuf, sizeof(cmdBuf), "V%02u", fan_pct);

	return send_command_wrapper(this, cmdBuf);

	struct uart_job *job;
	job = kmalloc(sizeof(*job), GFP_KERNEL);
	if (!job)
		return -ENOMEM;

	return 0;
}

static const struct fan_ctrl_ops wrapper_fc_ops = { .write_fan_speed =
							    write_fan };

struct syno_wrapper *
syno_wrapper_common_init(void *phy, const struct wrapper_phy_ops *phy_ops,
			 struct device *dev)
{
	struct syno_wrapper *priv;
	void *err;
	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (priv == NULL) {
		err = ERR_PTR(-ENOMEM);
		goto err0;
	}

	priv->dev = dev;
	priv->phy = phy;
	priv->phy_ops = phy_ops;

	priv->wq = alloc_ordered_workqueue("syno_wrapper", WQ_MEM_RECLAIM);
	if (priv->wq == NULL) {
		err = ERR_PTR(-ENOMEM);
		goto err_free_wrapper;
	}

	priv->wrapper_misc.minor = MISC_DYNAMIC_MINOR;
	priv->wrapper_misc.name = "syno_wrapper";
	priv->wrapper_misc.fops = &wrapper_fops;
	priv->wrapper_misc.mode = 0220; //ww-

	int result;
	result = misc_register(&priv->wrapper_misc);
	if (result) {
		err = ERR_PTR(result);
		goto err_destroy_wq;
	}

	struct fan_ctrl *wrapper_fc = fanctrl_create(
		(void *)priv, &wrapper_fc_ops, &ds918_curve, ds918_tz);

	if (IS_ERR(wrapper_fc)) {
		err = ERR_CAST(wrapper_fc);
		goto err_misc_dereg;
	}

	priv->fc = wrapper_fc;
	fanctrl_start(priv->fc);

	apollolake_gpios_table.dev_id = dev_name(dev);
	gpiod_add_lookup_table(&apollolake_gpios_table);
	struct bp_ctrl *wrapper_bp =
		backplanectrl_create(dev, "hdd-detect", "hdd-power");
	if (IS_ERR(wrapper_bp)) {
		err = ERR_CAST(wrapper_bp);
		goto err_bp_init;
	}

	priv->bp = wrapper_bp;
	backplanectrl_start(priv->bp);

	char cmd = SZ_UART_PWR_LED_OFF;
	send_command_wrapper(priv, &cmd);
	cmd = SZ_UART_STATUS_LED_OFF;
	send_command_wrapper(priv, &cmd);

	return priv;

err_bp_init:
	gpiod_remove_lookup_table(&apollolake_gpios_table);
	fanctrl_cleanup(priv->fc);
err_misc_dereg:
	misc_deregister(&priv->wrapper_misc);
err_destroy_wq:
	destroy_workqueue(priv->wq);
err_free_wrapper:
	kfree(priv);
err0:
	return err;
}
EXPORT_SYMBOL_GPL(syno_wrapper_common_init);

void syno_wrapper_common_cleanup(struct syno_wrapper *priv)
{
	backplanectrl_cleanup(priv->bp);
	gpiod_remove_lookup_table(&apollolake_gpios_table);
	fanctrl_cleanup(priv->fc);
	misc_deregister(&priv->wrapper_misc);
	flush_workqueue(priv->wq);
	destroy_workqueue(priv->wq);
	kfree(priv);
}
EXPORT_SYMBOL_GPL(syno_wrapper_common_cleanup);

MODULE_LICENSE("GPL");