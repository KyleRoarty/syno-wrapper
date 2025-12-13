#include <linux/module.h> /* Needed by all modules */
#include <linux/kernel.h> /* Needed for KERN_INFO */
#include <linux/gpio/consumer.h>

#include "arch/apollolake_common.h"
#include "include/syno_wrapper_common.h"
#include "include/fanctl.h"

struct uart_job {
	struct work_struct work;
	struct syno_wrapper *dev;
	u8 cmdBuf[SZ_UART_MAX_LENGTH+1];
};

static void send_command(struct work_struct *work) {
	struct uart_job *job = container_of(work, struct uart_job, work);
	printk(KERN_INFO "In light state toggle\n");

	int ret = job->dev->phy_ops->send_cmd(job->dev, job->cmdBuf);

	kfree(job);
}

static ssize_t wrapper_write(struct file * file, const char __user *buf, size_t count, loff_t * ppos)
{
	// I think this is correct?
	struct syno_wrapper *priv = container_of(file->private_data, struct syno_wrapper, wrapper_misc);
	struct uart_job *job;

	if (count > SZ_UART_MAX_LENGTH+1) {
		printk(KERN_INFO "Too much data - ignoring\n");
		*ppos += count;
		return count;
	}

	job = kmalloc(sizeof(*job), GFP_KERNEL);
	if (!job) {
		return -ENOMEM;
	}

	if (copy_from_user(job->cmdBuf, buf, count)) {
		return -EFAULT;
	}

	job->cmdBuf[count] = '\0';
	printk(KERN_INFO "Received: %s\n", job->cmdBuf);

	job->dev = priv;
	INIT_WORK(&job->work, send_command);

	queue_work(priv->wq, &job->work);

	*ppos += count;
	return count;
}

static const struct file_operations wrapper_fops = {
	.owner = THIS_MODULE,
	.write = wrapper_write
};

struct syno_wrapper *syno_wrapper_common_init(void *phy,
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

	priv->wq = alloc_ordered_workqueue("syno_wrapper", WQ_MEM_RECLAIM);
	if (priv->wq == NULL) {
		return ERR_PTR(-ENOMEM);
	}

	priv->wrapper_misc.minor = MISC_DYNAMIC_MINOR;
	priv->wrapper_misc.name = "syno_wrapper";
	priv->wrapper_misc.fops = &wrapper_fops;
	priv->wrapper_misc.mode = 0220; //ww-

	int result;
	result = misc_register(&priv->wrapper_misc);
	if (result) {
		destroy_workqueue(priv->wq);
		kfree(priv);
		return ERR_PTR(result);
	}

	gpiod_add_lookup_table(&apollolake_gpios_table);

	// Testing that CONFIG_XXX is tracked when building
	// Doesn't do anything yet
	enable_fanctrl();

	return priv;

	//power = gpiod_get(NULL, "power-led", GPIOD_OUT_LOW);
	//gpiod_set_value_cansleep(power, 0);

	return priv;
}
EXPORT_SYMBOL_GPL(syno_wrapper_common_init);

void syno_wrapper_common_cleanup(struct syno_wrapper *priv)
{
	misc_deregister(&priv->wrapper_misc);

	flush_workqueue(priv->wq);
	destroy_workqueue(priv->wq);
	//gpiod_set_value_cansleep(power, 1);
	//gpiod_put(power);
	gpiod_remove_lookup_table(&apollolake_gpios_table);
	kfree(priv);
	printk(KERN_INFO "Goodbye world 1.\n");
}
EXPORT_SYMBOL_GPL(syno_wrapper_common_cleanup);

MODULE_LICENSE("GPL");