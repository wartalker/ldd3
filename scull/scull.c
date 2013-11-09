/*
 * =====================================================================================
 *
 *       Filename:  scull.c
 *
 *    Description:  
 *
 *        Version:  1.0
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
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>

MODULE_LICENSE("Dual BSD/GPL");

struct scull_dev {
	char * data;
	unsigned long size;
	struct semaphore sem;
	struct cdev cdev;
};


int scull_major = 0;
int scull_minor = 0;

static struct scull_dev *sdev;

static int scull_open(struct inode *inode, struct file *filp)
{
	struct scull_dev *dev;

	dev = container_of(inode->i_cdev, struct scull_dev, cdev);
	filp->private_data = dev;

	return 0;
}


static int scull_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t scull_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct scull_dev *dev = filp->private_data;
	ssize_t ret = 0;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	if (*f_pos > dev->size)
		goto final;

	if (*f_pos + count > dev->size)
		count = dev->size - *f_pos;

	if (copy_to_user(buf, dev->data + *f_pos, count)) {
		ret = -EFAULT;
		goto final;
	}

	*f_pos += count;
	ret = count;

final:
	up(&dev->sem);
	return ret;
}

static ssize_t scull_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct scull_dev *dev = filp->private_data;
	ssize_t ret = -ENOMEM;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	if (*f_pos >= dev->size)
		goto final;

	if (*f_pos + count > dev->size)
		count = dev->size - *f_pos;

	if (copy_from_user(dev->data + *f_pos, buf, count)) {
		ret = -EFAULT;
		goto final;
	}	

	*f_pos += count;
	ret = count;

final:
	up(&dev->sem);
	return ret;
}


struct file_operations scull_fops = {
	.owner = THIS_MODULE,
	.open = scull_open,
	.read = scull_read,
	.write = scull_write,
	.release = scull_release,
};

static int scull_reg(struct scull_dev *dev)
{
	int ret;
	dev_t dt = 0;

	ret = alloc_chrdev_region(&dt, scull_minor, 1, "scull");
	scull_major = MAJOR(dt);

	return ret;
}

static int scull_setup(struct scull_dev *dev)
{
	int err; 
	dev_t devno = MKDEV(scull_major, scull_minor);

	sema_init(&dev->sem, 1);

	cdev_init(&dev->cdev, &scull_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &scull_fops;
	err = cdev_add(&dev->cdev, devno, 1);

	return err;
}

static int scull_mem(struct scull_dev *dev)
{
	dev->data = kmalloc(256, GFP_KERNEL);
	if (NULL == dev->data)
		return -ENOMEM;
	memset(dev->data, 0, 256);
	dev->size = 256;

	return 0;
}


static ssize_t scull_proc_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	int len = 0;

	if (down_interruptible(&sdev->sem))
		return -ERESTARTSYS;

	if (0 < *f_pos)
		goto final;

	len = snprintf(buf, count, "%d\n", (int)sdev->size);
	*f_pos += len;

final:
	up(&sdev->sem);
	return len;
}

static ssize_t scull_proc_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	char *str, *dp, *endp;
	unsigned long n;
	int ret = -ENOMEM;

	if (down_interruptible(&sdev->sem))
		return -ERESTARTSYS;

	str = kmalloc(32, GFP_KERNEL);
	if (NULL == str)
		return ret;

	memset(str, 0, 32);
	if (32 < count || 0 != *f_pos) {
		ret = -EFAULT;
		goto fail;
	}

	if (copy_from_user(str, buf, count)) {
		ret = -EFAULT;
		goto fail;
	}

	endp = str + count;
	n = simple_strtoul(str, &endp, 10);

	if (1024 < n || 0 == n) {
		ret = -EFAULT;
		goto fail;
	}

	dp = kmalloc(n, GFP_KERNEL);
	if (NULL == dp)
		goto fail;

	memset(dp, 0, n);

	kfree(sdev->data);
	sdev->data = dp;
	sdev->size = n;
	*f_pos += count;
	ret = count;

fail:
	kfree(str);
	up(&sdev->sem);
	return ret;
}

static struct file_operations scull_proc_fops = {
	.read = scull_proc_read,
	.write = scull_proc_write,
};

static void scull_proc_init(void)
{
	proc_create("scull", 0666, NULL, &scull_proc_fops);
}

static int __init scull_init(void)
{
	int ret = 0;
	sdev = kmalloc(sizeof(*sdev), GFP_KERNEL);
	if (NULL == sdev)
		return -ENOMEM;

	memset(sdev, 0, sizeof(*sdev));

	if (scull_reg(sdev)) {
		ret = -EFAULT;
		goto fault;
	}

	if (scull_setup(sdev)) {
		ret = -EFAULT;
		goto fault;
	}

	if (scull_mem(sdev)) {
		ret = -ENOMEM;
		goto fault;
	}

	scull_proc_init();

	return ret;

fault:
	kfree(sdev);
	return ret;
}


static void __exit scull_exit(void)
{
	dev_t devno;

        devno = MKDEV(scull_major, scull_minor);
	unregister_chrdev_region(devno, 1);

	kfree(sdev->data);
	kfree(sdev);

	remove_proc_entry("scull", NULL);
}

module_init(scull_init);
module_exit(scull_exit);
