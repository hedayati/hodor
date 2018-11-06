#ifndef _LINUX_HODOR_H
#define _LINUX_HODOR_H

#include <linux/types.h>
#include <linux/list.h>

struct hodor_opts;

typedef bool (*deny_signal_t)(void);

struct hodor_opts {
	deny_signal_t	deny_signal;
	void			*private;
};

extern bool hodor_deny_signal(void);
extern int register_hodor_opts(struct hodor_opts *p);
extern void unregister_hodor_opts(struct hodor_opts *p);

#endif /* _LINUX_UPROBES_H */