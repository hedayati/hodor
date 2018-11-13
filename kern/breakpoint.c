#include <linux/device.h>
#include <linux/fs.h>
#include <linux/hw_breakpoint.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kvm_para.h>
#include <linux/module.h>
#include <linux/perf_event.h>
#include <linux/ptrace.h>
#include <linux/sched/task_stack.h>
#include <linux/tlbpoison.h>

#include <asm/cacheflush.h>
#include <asm/debugreg.h>
#include <asm/tlbflush.h>

#include "hodor.h"

static void signal_br_handler(struct perf_event *bp,
                              struct perf_sample_data *data,
                              struct pt_regs *regs) {
  unsigned i;
  struct task_struct *tsk = current;
  struct thread_struct *thread = &tsk->thread;
  struct mm_struct *mm = tsk->mm;
  struct hodor_tls *tls = regs->hodor;
  struct hodor_config *config = tls->config;

  for (i = 0; i < HBP_NUM; i++) {
    if (thread->ptrace_bps[i] == bp) break;
  }

  printk(KERN_ALERT "Hodor: allowing signal...\n");

  spin_lock_irq(&tsk->sighand->siglock);
  recalc_sigpending(); /* see hodor_deny_signal() */
  spin_unlock_irq(&tsk->sighand->siglock);

  thread->debugreg6 &= ~(DR_TRAP0 << i);
}

static void inspection_br_handler(struct perf_event *bp,
                                  struct perf_sample_data *data,
                                  struct pt_regs *regs) {
  unsigned i;
  struct task_struct *tsk = current;
  struct thread_struct *thread = &tsk->thread;
  struct mm_struct *mm = tsk->mm;
  struct hodor_tls *tls = regs->hodor;
  struct hodor_config *config = tls->config;

  for (i = 0; i < HBP_NUM; i++) {
    if (thread->ptrace_bps[i] == bp) break;
  }

  if (config->inspect_count >= i &&
      config->inspect_begin_va[i] == bp->attr.bp_addr) {
    char *text = config->inspect_begin_va[i];
    if (text[0] == '\x0F' && text[1] == '\x01' && text[2] == '\xEF') {
      if (regs->ax != 0x55555554) {
        siginfo_t info;

        printk(KERN_ALERT
               "Hodor: execution of wrpkru %lx detected. Terminating...\n",
               regs->ax);

        info.si_signo = SIGSEGV;
        info.si_errno = 0;
        info.si_code = SEGV_PKUERR;
        info.si_addr = (void __user *)config->inspect_begin_va[i];
        info.si_pkey = regs->ax;
        send_sig_info(SIGSEGV, &info, tsk);
        return;
      } else {
        printk(KERN_INFO "Hodor: benign execution of wrpkru %lx detected.\n",
               regs->ax);
      }
    }

    thread->debugreg6 &= ~(DR_TRAP0 << i);
  }
}

static int arch_bp_generic_len(int x86_len) {
  switch (x86_len) {
    case X86_BREAKPOINT_LEN_1:
      return HW_BREAKPOINT_LEN_1;
    case X86_BREAKPOINT_LEN_2:
      return HW_BREAKPOINT_LEN_2;
    case X86_BREAKPOINT_LEN_4:
      return HW_BREAKPOINT_LEN_4;
#ifdef CONFIG_X86_64
    case X86_BREAKPOINT_LEN_8:
      return HW_BREAKPOINT_LEN_8;
#endif
    default:
      return -EINVAL;
  }
}

int arch_bp_generic_fields(int x86_len, int x86_type, int *gen_len,
                           int *gen_type) {
  int len;

  /* Type */
  switch (x86_type) {
    case X86_BREAKPOINT_EXECUTE:
      if (x86_len != X86_BREAKPOINT_LEN_X) return -EINVAL;

      *gen_type = HW_BREAKPOINT_X;
      *gen_len = sizeof(long);
      return 0;
    case X86_BREAKPOINT_WRITE:
      *gen_type = HW_BREAKPOINT_W;
      break;
    case X86_BREAKPOINT_RW:
      *gen_type = HW_BREAKPOINT_W | HW_BREAKPOINT_R;
      break;
    default:
      return -EINVAL;
  }

  /* Len */
  len = arch_bp_generic_len(x86_len);
  if (len < 0) return -EINVAL;
  *gen_len = len;

  return 0;
}

static int fill_bp_fields_helper(struct perf_event_attr *attr, int len,
                                 int type, bool disabled) {
  int err, bp_len, bp_type;

  err = arch_bp_generic_fields(len, type, &bp_len, &bp_type);
  if (!err) {
    attr->bp_len = bp_len;
    attr->bp_type = bp_type;
    attr->disabled = disabled;
  }

  return err;
}

static struct perf_event *register_instr_inspection_breakpoint(
    struct task_struct *tsk, int len, int type, unsigned long addr,
    bool disabled) {
  struct perf_event_attr attr;
  int err;

  ptrace_breakpoint_init(&attr);
  attr.bp_addr = addr;

  err = fill_bp_fields_helper(&attr, len, type, disabled);
  if (err) return ERR_PTR(err);

  return register_user_hw_breakpoint(&attr, inspection_br_handler, NULL, tsk);
}

int init_task_instr_inspection(void) {
  unsigned ret = 0, i, j;
  struct task_struct *tsk = current;
  struct thread_struct *thread = &tsk->thread;
  struct pt_regs *regs = task_pt_regs(tsk);
  struct mm_struct *mm = tsk->mm;
  struct hodor_tls *tls = regs->hodor;
  struct hodor_config *config = tls->config;

  if (config->inspect_count > 3) {
    /*
     * TODO: The page poisoning needs some work to avoid races. Until then,
     * let's not allow more than 3 illegal instructions (which already is more
     * than enough.)
     */
    printk(KERN_ALERT
           "Hodor: only up to 3 instruction inspection supported.\n");

    ret = -EINVAL;
    goto out;
  }

  for (i = 0; i < config->inspect_count; ++i) {
    unsigned long addr = config->inspect_begin_va[i];
    struct perf_event *bp = register_instr_inspection_breakpoint(
        tsk, X86_BREAKPOINT_LEN_X, X86_BREAKPOINT_EXECUTE, addr, false);

    if (IS_ERR(bp)) {
      printk(KERN_ALERT
             "Hodor: failed to register inspection watchpoints on %lx",
             addr);

      ret = -EINVAL;
      goto out;
    } else
      thread->ptrace_bps[i] = bp;
  }

out:
  return ret;
}

static struct perf_event *register_signal_breakpoint(struct task_struct *tsk,
                                                     int len, int type,
                                                     unsigned long addr,
                                                     bool disabled) {
  struct perf_event_attr attr;
  int err;

  ptrace_breakpoint_init(&attr);
  attr.bp_addr = addr;

  err = fill_bp_fields_helper(&attr, len, type, disabled);
  if (err) return ERR_PTR(err);

  return register_user_hw_breakpoint(&attr, signal_br_handler, NULL, tsk);
}

int arm_exit_tramp(unsigned long addr) {
  unsigned ret = 0, i, j;
  struct task_struct *tsk = current;
  struct thread_struct *thread = &tsk->thread;
  struct pt_regs *regs = task_pt_regs(tsk);
  struct mm_struct *mm = tsk->mm;
  struct hodor_tls *tls = regs->hodor;
  struct hodor_config *config = tls->config;

  struct perf_event *bp = register_signal_breakpoint(
      tsk, X86_BREAKPOINT_LEN_X, X86_BREAKPOINT_EXECUTE, addr, false);

  if (IS_ERR(bp)) {
    printk(KERN_ALERT "Hodor: failed to register signal watchpoints on %lx",
           addr);

    ret = -EINVAL;
    goto out;
  } else
    thread->ptrace_bps[3] = bp;

  /*
   * TODO: arm a timer so that we don't wait forever.
   */

out:
  return ret;
}
