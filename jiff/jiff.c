#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/jiffies.h>
#include <asm/msr.h>

MODULE_LICENSE("Dual BSD/GPL");

static ssize_t jiff_proc_read(struct file *filp, char __user *buf, 
		size_t count, loff_t *f_pos)
{
	int len = 0;
	u64 jiff;

//      jiff = get_jiffies_64();
	rdtscll(jiff);

	if (0 < *f_pos)
		return 0;

	len = snprintf(buf, count, "%llu\n", jiff);
	*f_pos += len;

	return len;
}

static struct file_operations jiff_proc_fops = {
	.read = jiff_proc_read,
};

static void jiff_proc_init(void)
{
	proc_create("jiff", 0666, NULL, &jiff_proc_fops);
}

static void jiff_proc_exit(void)
{
	remove_proc_entry("jiff", NULL);
}

static int __init jiff_init(void)
{
	jiff_proc_init();
	return 0;
}

static void __exit jiff_exit(void)
{
	jiff_proc_exit();
}

module_init(jiff_init);
module_exit(jiff_exit);
