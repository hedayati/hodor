#include <linux/compat.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/hodor.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/mm_types.h>
#include <linux/module.h>
#include <linux/perf_event.h>
#include <linux/sched/sysctl.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/types.h>

#include <asm/mman.h>

#include "hodor.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohammad Hedayati");
MODULE_DESCRIPTION("Hodor Driver");
MODULE_VERSION("1.0");

struct hodor_opts opts;

static int hodor_config(struct hodor_config *uconf) {
  unsigned ret = 0;
  struct task_struct *tsk = current;
  struct pt_regs *regs = task_pt_regs(tsk);
  struct mm_struct *mm = tsk->mm;
  struct hodor_config *config = NULL;
  struct hodor_tls *tls = regs->hodor;

  if (tls) {
    printk(KERN_ERR "Hodor: session already configured.\n");

    ret = -EEXIST;
    goto out;
  }

  config = vm_mmap(NULL, 0, PAGE_SIZE, PROT_READ | PROT_WRITE,
                   MAP_ANONYMOUS | MAP_PRIVATE, 0);
  if (!config) {
    printk(KERN_ERR "Hodor: failed to allocate configuration page.\n");

    ret = -ENOMEM;
    goto out;
  }

  /*
   * TODO: mark config page inaccessible by the user.
   */

  tls = vm_mmap(NULL, 0, PAGE_SIZE, PROT_READ | PROT_WRITE,
                MAP_ANONYMOUS | MAP_PRIVATE, 0);
  if (!tls) {
    printk(KERN_ERR "Hodor: failed to allocate TLS page.\n");

    ret = -ENOMEM;
    goto free_config;
  }

  /*
   * TODO: mark tls as read-only by the user.
   */

  printk(
      KERN_INFO
      "Hodor: session configured successfully (%d regions, %d inspection).\n",
      uconf->region_count, uconf->inspect_count);

  tls->config = config;

  memcpy(config, uconf, sizeof(struct hodor_config));

  regs->hodor = tls;
  return 0;

free_config:
  vm_munmap(config, PAGE_SIZE);

out:
  return ret;
}

static void sig_lockup_handler(struct timer_list *t) {
  struct hodor_tls *tls = from_timer(tls, t, sig_lockup_handler);
  struct task_struct *tsk = tls->tsk;

  printk(KERN_ALERT "Hodor: signal has been delayed for too long.\n");

  spin_lock_irq(&tsk->sighand->siglock);
  recalc_sigpending(); /* see hodor_deny_signal() */
  spin_unlock_irq(&tsk->sighand->siglock);
}

static int hodor_enter(void) {
  unsigned ret = 0;
  struct task_struct *tsk = current;
  struct pt_regs *regs = task_pt_regs(tsk);
  struct mm_struct *mm = tsk->mm;
  struct hodor_config *config = NULL;
  struct hodor_tls *tls = regs->hodor;

  if (!tls) {
    printk(KERN_ERR "Hodor: session should be configured before entering.\n");

    ret = -EINVAL;
    goto out;
  }
  config = tls->config;

  if (tls->tsk == tsk) {
    printk(KERN_ERR "Hodor: this task (pid: %d) has already entered.\n",
           tsk->pid);

    ret = -EEXIST;
    goto out;
  }

  if (tls->tsk) {
    tls = vm_mmap(NULL, 0, PAGE_SIZE, PROT_READ | PROT_WRITE,
                  MAP_ANONYMOUS | MAP_PRIVATE, 0);

    /*
     * TODO: mark tls as read-only by the user.
     */

    tls->config = config;
  }
  tls->tsk = tsk;
  tls->status_page = vm_mmap(NULL, 0, PAGE_SIZE, PROT_READ | PROT_WRITE,
                             MAP_ANONYMOUS | MAP_PRIVATE, 0);
  timer_setup(&tls->sig_lockup_handler, sig_lockup_handler, 0);

  regs->hodor = tls;

  return init_task_instr_inspection();

out:
  return ret;
}

static long hodor_dev_ioctl(struct file *filp, unsigned int ioctl,
                            unsigned long arg) {
  long r = -EINVAL;
  struct task_struct *tsk = current;
  struct pt_regs *regs = task_pt_regs(tsk);

  struct hodor_config conf;
  struct hodor_tls tls;

  switch (ioctl) {
    case HODOR_CONFIG:
      r = copy_from_user(&conf, (int __user *)arg, sizeof(struct hodor_config));
      if (r) {
        r = -EIO;
        goto out;
      }

      r = hodor_config(&conf);
      break;
    case HODOR_ENTER:
      r = hodor_enter();
      break;
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

bool hodor_deny_signal(void) {
  unsigned i;
  struct task_struct *tsk = current;
  struct pt_regs *regs = task_pt_regs(tsk);
  struct hodor_tls *tls = regs->hodor;
  struct hodor_config *config = NULL;

  if (!tls) return false;
  config = tls->config;

  if (tls->sig_delayed) {
    printk(KERN_ALERT "not delaying double signal ip: %lx region: %d\n",
           regs->ip, i);
    tls->sig_delayed = false;
    return false;
  }

  for (i = 0; i < config->region_count; ++i) {
    if (!config->region_pkey[i] || !config->exit_tramp_va[i]) continue;
    if (regs->ip >= config->region_begin_va[i] &&
        regs->ip <= config->region_begin_va[i] + config->region_len[i]) {
      printk(KERN_ALERT "delaying signal ip: %lx region: %d\n", regs->ip, i);
      if (signal_pending(tsk)) {
        spin_lock_irq(&tsk->sighand->siglock);
        clear_tsk_thread_flag(tsk, TIF_SIGPENDING);
        spin_unlock_irq(&tsk->sighand->siglock);

        arm_exit_tramp(config->exit_tramp_va[i]);
        tls->sig_delayed = true;
        return true;
      }
    }
  }

  return false;
}

static int __init hodor_init(void) {
  int r;

  printk(KERN_INFO "Hodor module loaded\n");

  r = misc_register(&hodor_dev);
  if (r) printk(KERN_ERR "Hodor: misc device register failed\n");

  opts.private = NULL;
  opts.deny_signal = hodor_deny_signal;
  register_hodor_opts(&opts);

  return r;
}

static void __exit hodor_exit(void) {
  misc_deregister(&hodor_dev);
  unregister_hodor_opts(&opts);
}

module_init(hodor_init);
module_exit(hodor_exit);
