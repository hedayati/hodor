#pragma once

#include <linux/types.h>

#define HODOR_MINOR 233

struct hodor_config;
struct hodor_tls;

#define HODOR_CONFIG _IOR(HODOR_MINOR, 0x01, struct hodor_config)
#define HODOR_ENTER _IOR(HODOR_MINOR, 0x02, struct hodor_tls)

#define HODOR_SCRATCH_AREA ((unsigned long *)0xffffffffff578000)
#define HODOR_REG ((unsigned long *)0xffffffffff578ff0)

struct hodor_config {
  unsigned int region_count;
  unsigned long region_begin_va[16];
  unsigned long region_len[16];
  unsigned int region_pkey[16];
  unsigned long exit_tramp_va[16];

  unsigned int inspect_count;
  unsigned long inspect_begin_va[16];
};

struct hodor_tls {
  struct task_struct *tsk;
  struct hodor_config *config;
  unsigned long ustack;
  unsigned long pstack;
};

int init_task_instr_inspection(void);
int arm_exit_tramp(unsigned long ip);
