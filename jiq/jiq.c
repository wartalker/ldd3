/*
 * =====================================================================================
 *
 *       Filename:  jiq_pipe.c
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
#include <linux/proc_fs.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

MODULE_LICENSE("Dual BSD/GPL");

static DECLARE_WAIT_QUEUE_HEAD (jiq_wait);

struct jiq_info {
	char *buf;
	size_t size;
	size_t len;
	unsigned long jiff;
	struct delayed_work work;
};

static struct jiq_info jiq_data;

static void jiq_print(struct work_struct *w)
{
	size_t len = 0;
	unsigned long j = jiffies;
	struct delayed_work *dw = container_of(w, struct delayed_work, work);
	struct jiq_info *p = container_of(dw, struct jiq_info, work);

	len = snprintf(p->buf + p->len, 256 - p->len, "%9lu  %4li     %3i %5i %3i %s\n", j, j - p->jiff, 
			preempt_count(), current->pid, smp_processor_id(), current->comm);
	p->len += len;

	wake_up_interruptible(&jiq_wait);

	return;
}

static ssize_t jiq_proc_read(struct file *filp, char __user *buf, 
		size_t count, loff_t *f_pos)
{
	ssize_t ret = 0;
	DEFINE_WAIT(wait);

	if (0 < *f_pos)
		return 0;

	jiq_data.buf = kmalloc(256, GFP_KERNEL);
	if (NULL == jiq_data.buf)
		return -ENOMEM;

	memset(jiq_data.buf, 0, 256);
	jiq_data.size = 256;
	jiq_data.jiff = jiffies;
	jiq_data.len = snprintf(jiq_data.buf, 256, "     time  delta preempt   pid cpu command\n");

	prepare_to_wait(&jiq_wait, &wait, TASK_INTERRUPTIBLE);
	schedule_delayed_work(&jiq_data.work, 100);
	schedule();
	finish_wait(&jiq_wait, &wait);

	if (copy_to_user(buf, jiq_data.buf, jiq_data.len)) {
		ret = -EFAULT;
		goto final;
	}

	*f_pos += jiq_data.len;
	ret = jiq_data.len;

final:
	kfree(jiq_data.buf);
	return ret;
}

static struct file_operations jiq_proc_fops = {
	.read = jiq_proc_read,
};

static void jiq_proc_init(void)
{
	proc_create("jiq", 0666, NULL, &jiq_proc_fops);
}

static void jiq_proc_exit(void)
{
	remove_proc_entry("jiq", NULL);
}

static int __init jiq_init(void)
{
	jiq_proc_init();
	INIT_DELAYED_WORK(&jiq_data.work, jiq_print);

	return 0;
}

static void __exit jiq_exit(void)
{
	jiq_proc_exit();
}

module_init(jiq_init);
module_exit(jiq_exit);
