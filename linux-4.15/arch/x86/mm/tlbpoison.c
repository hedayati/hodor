#include <linux/list.h>
#include <linux/rculist.h>
#include <linux/spinlock.h>
#include <linux/hash.h>
#include <linux/export.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/ptrace.h>
#include <linux/preempt.h>
#include <linux/percpu.h>
#include <linux/kdebug.h>
#include <linux/mutex.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <asm/cacheflush.h>
#include <asm/tlbflush.h>
#include <linux/errno.h>
#include <asm/debugreg.h>
#include <linux/tlbpoison.h>

static DEFINE_SPINLOCK(tlbpoison_lock);

/* Protected by tlbpoison_lock */
unsigned int tlbpoison_count;

static LIST_HEAD(tlbpoison_probes);

static struct tlbpoison_probe *get_tlbpoison_probe(struct mm_struct *mm, unsigned long addr)
{
	struct tlbpoison_probe *p;
	list_for_each_entry_rcu(p, &tlbpoison_probes, list) {
		if (mm == p->mm && addr == p->addr)
			return p;
	}
	return NULL;
}

int register_tlbpoison_probe(struct tlbpoison_probe *p) {
	unsigned long flags;
	int ret = 0;
	unsigned long addr = p->addr & PAGE_MASK;
	struct mm_struct *mm = p->mm;
	unsigned int l;
	pte_t *pte;

	spin_lock_irqsave(&tlbpoison_lock, flags);
	if (get_tlbpoison_probe(mm, addr)) {
		ret = -EEXIST;
		goto out;
	}

	pte = lookup_address_in_pgd(pgd_offset(mm, addr), addr, &l);
	if (!pte) {
		ret = -EINVAL;
		goto out;
	}

	tlbpoison_count++;
	p->addr = p->addr & PAGE_MASK;
	list_add_rcu(&p->list, &tlbpoison_probes);
out:
	spin_unlock_irqrestore(&tlbpoison_lock, flags);
	return ret;
}
EXPORT_SYMBOL(register_tlbpoison_probe);

void unregister_tlbpoison_probe(struct tlbpoison_probe *p) {
	unsigned long flags;
	unsigned long addr = p->addr & PAGE_MASK;
	struct mm_struct *mm = p->mm;
	unsigned int l;
	pte_t *pte;

	pte = lookup_address_in_pgd(pgd_offset(mm, addr), addr, &l);
	if (!pte)
		return;

	spin_lock_irqsave(&tlbpoison_lock, flags);
	list_del_rcu(&p->list);
	tlbpoison_count--;
	spin_unlock_irqrestore(&tlbpoison_lock, flags);
}
EXPORT_SYMBOL(unregister_tlbpoison_probe);

int tlbpoison_handler(struct pt_regs *regs, unsigned long addr)
{
	unsigned long flags;
	int ret = 0;
	struct mm_struct *mm = current->mm;
	unsigned int l;
	pte_t *pte;
	struct tlbpoison_probe *p = NULL;

	preempt_disable();
	rcu_read_lock();

	pte = lookup_address_in_pgd(pgd_offset(mm, addr), addr, &l);
	if (!pte) {
		ret = -EINVAL;
		goto no_tlbpoison;
	}

	p = get_tlbpoison_probe(mm, addr);
	if (!p) {
		ret = -EINVAL;
		goto no_tlbpoison;
	}

	p->handler(p, regs);

no_tlbpoison:
	rcu_read_unlock();
	preempt_enable_no_resched();
	return ret;
}
