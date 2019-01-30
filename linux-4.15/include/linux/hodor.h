#ifndef _LINUX_HODOR_H
#define _LINUX_HODOR_H

#include <linux/types.h>
#include <linux/list.h>

struct hodor_opts;

typedef bool (*deny_signal_t)(void);
typedef bool (*deny_mmap_t)(void *addr, size_t len, int prot, int flags,
                  struct file *file, vm_flags_t vm_flags, unsigned long pgoff);
typedef bool (*deny_mprotect_t)(void *addr, size_t len, int prot, int pkey);

struct hodor_opts {
	deny_signal_t	deny_signal;
	deny_mmap_t		deny_mmap;
	deny_mprotect_t	deny_mprotect;
	void			*private;
};

extern bool hodor_deny_signal(void);
extern bool hodor_deny_mmap(void *addr, size_t len, int prot, int flags,
                  struct file *file, vm_flags_t vm_flags, unsigned long pgoff);
extern bool hodor_deny_mprotect(void *addr, size_t len, int prot, int pkey);
extern int register_hodor_opts(struct hodor_opts *p);
extern void unregister_hodor_opts(struct hodor_opts *p);

#endif /* _LINUX_HODOR_H */