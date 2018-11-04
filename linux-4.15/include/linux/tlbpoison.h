/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_TLBPOISON_H
#define _LINUX_TLBPOISON_H

#include <linux/types.h>
#include <linux/list.h>

struct tlbpoison_probe;
struct pt_regs;

typedef void (*tlbpoison_handler_t)(struct tlbpoison_probe *,
				struct pt_regs *);

struct tlbpoison_probe {
	struct list_head	list;
	struct mm_struct	*mm;
	unsigned long		addr;
	tlbpoison_handler_t	handler;
	void				*private;
};

extern int register_tlbpoison_probe(struct tlbpoison_probe *p);
extern void unregister_tlbpoison_probe(struct tlbpoison_probe *p);

/* Called from page fault handler. */
extern int tlbpoison_handler(struct pt_regs *regs, unsigned long addr);

#endif /* _LINUX_TLBPOISON_H */