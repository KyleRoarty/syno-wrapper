#include <linux/serdev.h>
#include <linux/acpi.h>

#include "include/syno_wrapper_common.h"

struct uart_ctrl {
	struct serdev_device *serdev;
	void *priv;
};

static int wrapper_uart_write(void *phy, const u8 *cmd)
{
	struct uart_ctrl *uart = phy;
	u8 cmdbuf[32] = {0};
	// Could check for null here but yolo
	scnprintf(cmdbuf, sizeof(cmdbuf), "%s", cmd);

	// Might want to comment out if the fan ctrl
	// messages pollute this too much
	pr_info("uart command: %s\n", cmdbuf);

    int err = serdev_device_write(uart->serdev, cmdbuf,
		sizeof(cmdbuf), 0);
	return err;
	if (err < 0)
		return err;
	// Potentially do something here before returning
	// pn533 does something with a timer
	return 0;
}

static const struct wrapper_phy_ops uart_ops = {
	.send_cmd = wrapper_uart_write,
};

static size_t wrapper_uart_receive(struct serdev_device *serdev, const u8 *buffer, size_t size)
{
	pr_info("serdev echo: received %ld bytes: ", size);
	for (int i = 0; i < size; i++)
		pr_cont("0x%02X ", buffer[i]);
	pr_cont("\n");
	return size;
}

static const struct serdev_device_ops serdev_ops = {
	.receive_buf = wrapper_uart_receive,
	.write_wakeup = serdev_device_write_wakeup,
};

static int wrapper_uart_probe(struct serdev_device *serdev)
{
	pr_info("In probe for serdev\n");
	struct uart_ctrl *uart;
	void *priv;

	uart = kzalloc(sizeof(*uart), GFP_KERNEL);

	uart->serdev = serdev;
	serdev_device_set_drvdata(serdev, uart);
	serdev_device_set_client_ops(serdev, &serdev_ops);
	int status = serdev_device_open(serdev);
	if (status)
	{
		pr_info("serdev probe: error when opening serial device\n");
		return -1;
	}

	serdev_device_set_baudrate(serdev, 9600);
	serdev_device_set_parity(serdev, SERDEV_PARITY_NONE);
	serdev_device_set_flow_control(serdev, false);

	priv = syno_wrapper_common_init(uart,
		&uart_ops, &uart->serdev->dev);
	uart->priv = priv;
	return 0;
}

static void wrapper_uart_remove(struct serdev_device *serdev)
{
	struct uart_ctrl *uart = serdev_device_get_drvdata(serdev);
	syno_wrapper_common_cleanup(uart->priv);
	serdev_device_close(serdev);
	kfree(uart);
}

static const struct acpi_device_id uart_acpi_ids[] = {
	{"TST0001", 0},
	{}
};
MODULE_DEVICE_TABLE(acpi, uart_acpi_ids);

static struct serdev_device_driver uart_driver = {
	.driver =
		{
			.name = "syno-wrap",
			.acpi_match_table = uart_acpi_ids,
		},
	.probe = wrapper_uart_probe,
	.remove = wrapper_uart_remove
};
module_serdev_device_driver(uart_driver);

MODULE_LICENSE("GPL");
