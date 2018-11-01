#include <linux/compat.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/perf_event.h>
#include <linux/types.h>

#include "hodor.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohammad Hedayati");
MODULE_DESCRIPTION("Hodor Driver");
MODULE_VERSION("1.0");

static long hodor_dev_ioctl(struct file *filp, unsigned int ioctl,
                            unsigned long arg) {
  long r = -EINVAL;

  switch (ioctl) {
    default:
      return -ENOTTY;
  }

out:
  return r;
}

static int hodor_dev_release(struct inode *inode, struct file *file) {
  return 0;
}

static const struct file_operations hodor_chardev_ops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = hodor_dev_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = hodor_dev_ioctl,
#endif
    .llseek = noop_llseek,
    .release = hodor_dev_release,
};

static struct miscdevice hodor_dev = {
    HODOR_MINOR,
    "hodor",
    &hodor_chardev_ops,
};

static int __init hodor_init(void) {
  int r;

  printk(KERN_ERR "Hodor module loaded\n");

  r = misc_register(&hodor_dev);
  if (r) printk(KERN_ERR "Hodor: misc device register failed\n");

  return r;
}

static void __exit hodor_exit(void) { misc_deregister(&hodor_dev); }

module_init(hodor_init);
module_exit(hodor_exit);