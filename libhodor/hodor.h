#pragma once

#include <stdbool.h>
#include <sys/queue.h>

#include "../kern/hodor.h"

#define PAGE_SIZE 0x1000

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

extern int hodor_init(void);
extern int hodor_enter(struct hodor_tls *);
