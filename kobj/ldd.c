/*
 * =====================================================================================
 *
 *       Filename:  ldd_pipe.c
 *
 *    Description:  
 *
 *        Version:  3.1415
 *        Created:  11/04/2013 03:02:11 PM
 *
 *         Author:  wartalker (LiuWei), wartalker@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */


#include <linux/init.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/device.h>

MODULE_LICENSE("Dual BSD/GPL");


static int ldd_match(struct device *dev, struct device_driver *drv)
{
	return 1;
}

static void ldd_bus_release(struct device *dev)
{
}

static struct device ldd_bus = {
	.init_name = "ldd0",
	.release = ldd_bus_release,
};

static struct bus_type ldd_bus_type = {
	.name = "ldd",
	.match = ldd_match,
};

static int __init ldd_init(void)
{
	int ret = 0;

	ret = bus_register(&ldd_bus_type);

	if (ret)
		return ret;

	ret = device_register(&ldd_bus);

	return ret;
}


static void __exit ldd_exit(void)
{
	device_unregister(&ldd_bus);
	bus_unregister(&ldd_bus_type);
}

module_init(ldd_init);
module_exit(ldd_exit);
