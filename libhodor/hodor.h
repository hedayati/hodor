#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <sys/queue.h>

#include "../kern/hodor.h"

#define PAGE_SHIFT 12
#define PAGE_SIZE (1UL << PAGE_SHIFT)
#define PAGE_MASK (~(PAGE_SIZE - 1))

// process memory maps

#define PROCMAP_TYPE_UNKNOWN 0x00
#define PROCMAP_TYPE_FILE 0x01
#define PROCMAP_TYPE_ANONYMOUS 0x02
#define PROCMAP_TYPE_HEAP 0x03
#define PROCMAP_TYPE_STACK 0x04
#define PROCMAP_TYPE_VSYSCALL 0x05
#define PROCMAP_TYPE_VDSO 0x06
#define PROCMAP_TYPE_VVAR 0x07

struct dune_procmap_entry {
  unsigned long begin;
  unsigned long end;
  unsigned long offset;
  bool r;  // Readable
  bool w;  // Writable
  bool x;  // Executable
  bool p;  // Private (or shared)
  char *path;
  int type;
};

typedef void (*dune_procmap_cb)(const struct dune_procmap_entry *);

extern void dune_procmap_iterate(dune_procmap_cb cb);
extern void dune_procmap_dump();

struct tls {
  unsigned long stack;
};

#define TLSU ((struct tls *)((struct hodor_tls *)*HODOR_REG)->status_page_u)
#define TLSP ((struct tls *)((struct hodor_tls *)*HODOR_REG)->status_page_p)
#define PROTECTED_STACK_SIZE 16 * PAGE_SIZE

extern int hodor_init(void);
extern int hodor_enter(void);

#ifdef __cplusplus
}
#endif
