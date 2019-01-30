#include <linux/kernel.h>
#include <linux/highmem.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/sched/mm.h>
#include <linux/sched/coredump.h>
#include <linux/export.h>
#include <linux/percpu-rwsem.h>
#include <linux/task_work.h>
#include <linux/shmem_fs.h>

#include <linux/hodor.h>

static DEFINE_SPINLOCK(hodor_opts_lock);

struct hodor_opts *opts = NULL;

bool hodor_deny_signal(void) {
	if (!opts)
		return false;

	return opts->deny_signal();
}

extern bool hodor_deny_mmap(void *addr, size_t len, int prot, int flags,
                  struct file *file, vm_flags_t vm_flags, unsigned long pgoff) {
	if (!opts)
		return false;

	return opts->deny_mmap(addr, len, prot, flags, file, vm_flags, pgoff);
}

extern bool hodor_deny_mprotect(void *addr, size_t len, int prot, int pkey) {
	if (!opts)
		return false;

	return opts->deny_mprotect(addr, len, prot, pkey);
}

int register_hodor_opts(struct hodor_opts *p) {
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(&hodor_opts_lock, flags);
	if (opts) {
		ret = -EEXIST;
		goto out;
	}

	opts = p;
out:
	spin_unlock_irqrestore(&hodor_opts_lock, flags);
	return ret;
}
EXPORT_SYMBOL(register_hodor_opts);

void unregister_hodor_opts(struct hodor_opts *p) {
	unsigned long flags;

	spin_lock_irqsave(&hodor_opts_lock, flags);
	if (opts == p)
		opts = NULL;
	spin_unlock_irqrestore(&hodor_opts_lock, flags);
}
EXPORT_SYMBOL(unregister_hodor_opts);
