#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>

MODULE_LICENSE("Dual BSD/GPL");


static int ldd_match(struct device *dev, struct device_driver *drv)
{
	return !strncmp(dev->init_name, drv->name, strlen(drv->name));
}

static int ldd_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	env->envp[0] = NULL;
	return 0;
}

static struct bus_type ldd_bus_type = {
	.name = "ldd",
	.match = ldd_match,
	.uevent = ldd_uevent,
};

static void ldd_bus_release(struct device *dev)
{
	printk(KERN_DEBUG "ldd bus released!");
}

static struct device ldd_bus = {
	.init_name = "ldd0",
	.release = ldd_bus_release,
};

static int __init ldd_init(void)
{
	int ret = 0;

	ret = bus_register(&ldd_bus_type);

	if (ret)
		return ret;

	return device_register(&ldd_bus);
}


static void __exit ldd_exit(void)
{
	device_unregister(&ldd_bus);
	bus_unregister(&ldd_bus_type);
}

module_init(ldd_init);
module_exit(ldd_exit);


struct ldd_driver {
	char *version;
	struct module *mod;
	struct device_driver drv;
	struct device_attribute attr;
};

struct ldd_device {
	char *name;
	struct ldd_driver *drv;
	struct device dev;
	struct device_attribute attr;
};

static void ldd_device_release(struct device *dev)
{
}

int register_ldd_device(struct ldd_device *ldddev)
{
	ldddev->dev.bus = &ldd_bus_type;
	ldddev->dev.parent = &ldd_bus;
	ldddev->dev.release = ldd_device_release;
	strncpy((char *)ldddev->dev.init_name, ldddev->name, 20);
	return device_register(&ldddev->dev);
}
EXPORT_SYMBOL(register_ldd_device);

void unregister_ldd_device(struct ldd_device *ldddev)
{
	return device_unregister(&ldddev->dev);
}
EXPORT_SYMBOL(unregister_ldd_device);

int register_ldd_driver(struct ldd_driver *ldddrv)
{
	ldddrv->drv.bus = &ldd_bus_type;
	return driver_register(&ldddrv->drv);
}
EXPORT_SYMBOL(register_ldd_driver);

void unregister_ldd_driver(struct ldd_driver *ldddrv)
{
	return driver_unregister(&ldddrv->drv);
}
EXPORT_SYMBOL(unregister_ldd_driver);
