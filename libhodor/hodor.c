#define _GNU_SOURCE
#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <link.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <linux/types.h>

#include "hodor.h"
#include "inst.h"

int hodor_fd = -1;
int pkey;
struct hodor_config config;

struct __hodor_func {
  char *sym;
  unsigned argc;
};

char *hodor_plib_path = NULL;
char *hodor_plib_init_func = NULL;
struct __hodor_func hodor_plib_funcs[1024];
unsigned hodor_plib_funcs_count = 0;
typedef int (*hodor_plib_init_t)(void);

static void hodor_insert_trampoline(char *func_name, char *func_text,
                                    char *tramp_text,
                                    unsigned long *tramp_idx) {
  unsigned long i;
  unsigned long func_text_idx = 0;

  // 48 8d a4 24 00 00 00 00    lea    0x0(%rsp),%rsp
  if (func_text[0] != '\x48' || func_text[1] != '\x8d' ||
      func_text[2] != '\xa4' || func_text[3] != '\x24' ||
      func_text[4] != '\x00' || func_text[5] != '\x00' ||
      func_text[6] != '\x00' || func_text[7] != '\x00') {
    printf("libhodor: HODOR_FUNC_ATTR missing from %s.\n", func_name);
    exit(-EINVAL);
  }

  /*
   * movabs $tramp, %rax
   * jmpq   *%rax
   */
  x86_inst_jmpq_rel32(func_text, &func_text_idx,
                      (unsigned long)&tramp_text[*tramp_idx]);

  /*
   * ********************** BOUNDARY_TRAMP *********************
   * retq                         c3
   * ************************ TRAMPOLINE ***********************
   * save source %rsp
   * flip protection
   * restore destination %rsp
   * movabs $(text + 32), %rax
   * callq  %rax
   * flip protection again
   * restore source %rsp
   * jmpq   $BOUNDARY_TRAMP
   * ********************* NEXT TRAMPOLINE *********************
   */

  x86_inst_movabs_rax(tramp_text, tramp_idx, HODOR_REG);
  x86_inst_mov_atrax_rax(tramp_text, tramp_idx);  // (%rax) points to kernel_tls
  x86_inst_mov_atrax_rax(tramp_text,
                         tramp_idx);  // (%rax) points to unprotected_tls
  x86_inst_mov_rsp_atrax(tramp_text,
                         tramp_idx);  // unprotected_tls->stack = %rsp

  x86_inst_push_rcx(tramp_text, tramp_idx);
  x86_inst_push_rdx(tramp_text, tramp_idx);
  x86_inst_xor_ecx_ecx(tramp_text, tramp_idx);
  x86_inst_xor_edx_edx(tramp_text, tramp_idx);
  unsigned long lbl0 = (unsigned long)&tramp_text[*tramp_idx];
  // x86_inst_rdpkru(tramp_text, tramp_idx);
  x86_inst_mov_eax(tramp_text, tramp_idx, 0x55555550);
  x86_inst_wrpkru(tramp_text, tramp_idx);
  x86_inst_cmp_eax(tramp_text, tramp_idx, 0x55555550);
  x86_inst_jne_rel8(tramp_text, tramp_idx, lbl0);
  x86_inst_pop_rdx(tramp_text, tramp_idx);
  x86_inst_pop_rcx(tramp_text, tramp_idx);

  x86_inst_movabs_rax(tramp_text, tramp_idx, HODOR_REG);
  x86_inst_mov_atrax_rax(tramp_text, tramp_idx);  // (%rax) points to kernel_tls
  x86_inst_mov_atrax_rax_off8(tramp_text, tramp_idx,
                              0x8);  // (%rax) points to protected_tls
  x86_inst_mov_atrax_rsp(tramp_text, tramp_idx);  // %rsp = protected_tls->stack

  x86_inst_movabs_rax(tramp_text, tramp_idx, (unsigned long)func_text + 8);
  x86_inst_callq_rax(tramp_text, tramp_idx);

  x86_inst_push_rax(tramp_text, tramp_idx);
  x86_inst_push_rcx(tramp_text, tramp_idx);
  x86_inst_push_rdx(tramp_text, tramp_idx);
  x86_inst_xor_ecx_ecx(tramp_text, tramp_idx);
  x86_inst_xor_edx_edx(tramp_text, tramp_idx);
  unsigned long lbl1 = (unsigned long)&tramp_text[*tramp_idx];
  // x86_inst_rdpkru(tramp_text, tramp_idx);
  x86_inst_mov_eax(tramp_text, tramp_idx, 0x55555558);
  x86_inst_wrpkru(tramp_text, tramp_idx);
  x86_inst_cmp_eax(tramp_text, tramp_idx, 0x55555558);
  x86_inst_jne_rel8(tramp_text, tramp_idx, lbl1);
  x86_inst_pop_rdx(tramp_text, tramp_idx);
  x86_inst_pop_rcx(tramp_text, tramp_idx);
  x86_inst_pop_rax(tramp_text, tramp_idx);

  x86_inst_movabs_r10(tramp_text, tramp_idx, HODOR_REG);
  x86_inst_mov_atr10_r10(tramp_text, tramp_idx);  // (%rax) points to kernel_tls
  x86_inst_mov_atr10_r10(tramp_text,
                         tramp_idx);  // (%rax) points to unprotected_tls
  x86_inst_mov_atr10_rsp(tramp_text,
                         tramp_idx);  // %rsp = unprotected_tls->stack

  x86_inst_jmpq_rel32(tramp_text, tramp_idx, (unsigned long)tramp_text);
}

static void __setup_mappings_cb(const struct dune_procmap_entry *ent) {
  if (ent->x) {
    unsigned long i;
    char *text;
    unsigned index = config.region_count;
    bool is_plib = (hodor_plib_path && strcmp(ent->path, hodor_plib_path) == 0);
    void *plib_handle = NULL;
    char *tramp_text = NULL;
    unsigned long tramp_idx = 0;

    if (is_plib) {
      plib_handle = dlopen(ent->path, RTLD_LAZY);
      if (!plib_handle) {
        printf("libhodor: failed to open handle for %s\n", ent->path);
        exit(-EINVAL);
      }

      // Each trampoline is roughly 128 bytes.
      tramp_text = mmap(
          NULL, (1 + (hodor_plib_funcs_count * 128) / PAGE_SIZE) * PAGE_SIZE,
          PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1,
          0);
      if (!tramp_text) {
        printf("libhodor: failed allocating trampoline pages for %s\n",
               ent->path);
        exit(-ENOMEM);
      }

      x86_inst_retq(tramp_text, &tramp_idx);
    }

    config.region_begin_va[index] = ent->begin;
    config.region_len[index] = ent->end - ent->begin;
    config.region_pkey[index] = is_plib ? 1 : 0;
    config.exit_tramp_va[index] = (unsigned long)tramp_text;

    text = (char *)ent->begin;
    for (i = 0; i < config.region_len[index] - 3; ++i) {
      if (text[i] == '\x0f' && text[i + 2] == '\xef' && text[i + 1] == '\x01') {
        config.inspect_begin_va[config.inspect_count++] =
            (unsigned long)&text[i];
      }
    }

    if (is_plib) {
      mprotect((void *)ent->begin, ent->end - ent->begin,
               PROT_READ | PROT_WRITE);

      for (i = 0; i < hodor_plib_funcs_count; ++i) {
        char *func_text = dlsym(plib_handle, hodor_plib_funcs[i].sym);
        if (!func_text) {
          printf(
              "libhodor: failed finding function %s from %s (is it mangled? if "
              "so, use extern \"C\")\n",
              hodor_plib_funcs[i].sym, ent->path);
          exit(-EINVAL);
        }

        hodor_insert_trampoline(hodor_plib_funcs[i].sym, func_text, tramp_text,
                                &tramp_idx);
      }

      mprotect((void *)ent->begin, ent->end - ent->begin,
               PROT_READ | PROT_EXEC);
      mprotect((void *)tramp_text, PAGE_SIZE, PROT_READ | PROT_EXEC);

      hodor_plib_init_t init_func = dlsym(plib_handle, hodor_plib_init_func);
      if (!init_func) {
        printf("libhodor: failed finding initialization from %s\n", ent->path);
        exit(-EINVAL);
      }
      init_func();
    }

    config.region_count++;
  }
}

static inline bool strstarts(const char *pre, const char *str) {
  size_t lenpre = strlen(pre), lenstr = strlen(str);
  return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}

static int elf_symtab_callback(struct dl_phdr_info *info, size_t size,
                               void *data) {
  unsigned i;

  if (strstr(info->dlpi_name, "vdso")) return 0;

  for (i = 0; i < info->dlpi_phnum; i++) {
    if (info->dlpi_phdr[i].p_type == PT_DYNAMIC) {
      ElfW(Dyn *) dyn =
          (ElfW(Dyn) *)(info->dlpi_addr + info->dlpi_phdr[i].p_vaddr);

      /*
       * Iterate over all entries of the dynamic section until we find the
       * string table (DT_STRTAB).
       */
      while (dyn && dyn->d_tag != DT_NULL) {
        if (dyn->d_tag == DT_STRTAB) {
          unsigned i = 1;
          /* Get the pointer to the string table */
          char *strtab = (char *)dyn->d_un.d_ptr;
          while (strtab) {
            char *str = &strtab[i];
            if (strstarts("__hodor_init_", str)) {
              if (hodor_plib_path) {
                printf(
                    "libhodor: [%s] is already identified as a protected "
                    "library. [%s] cannot be a protected library.\n",
                    hodor_plib_path, info->dlpi_name);

                exit(-EINVAL);
              }

              hodor_plib_path = malloc(PATH_MAX);
              realpath(info->dlpi_name, hodor_plib_path);

              hodor_plib_init_func = malloc(strlen(str));
              strcpy(hodor_plib_init_func, str + 13);
            } else if (strstarts("__hodor_func_", str)) {
              hodor_plib_funcs[hodor_plib_funcs_count].sym =
                  malloc(strlen(str));
              strcpy(hodor_plib_funcs[hodor_plib_funcs_count].sym, str + 13);
              hodor_plib_funcs_count++;
            }
            unsigned len = strlen(str);
            if (!len) break;
            i += len + 1;
          }

          i = 1;
          while (strtab) {
            char *str = &strtab[i];
            if (strstarts("__hodor_argc_", str)) {
              bool found = false;
              int j;
              char *substr = malloc(strlen(str));
              strcpy(substr, str + 13);

              for (j = 0; j < hodor_plib_funcs_count; ++j) {
                if (strstarts(hodor_plib_funcs[j].sym, substr)) {
                  strcpy(substr, substr + strlen(hodor_plib_funcs[j].sym) + 1);
                  hodor_plib_funcs[j].argc = atoi(substr); /* FIX IT */
                  if (hodor_plib_funcs[j].argc > 6) {
                    printf(
                        "libhodor: function %s has more than 6 arguments "
                        "(which we don't support yet!)\n",
                        hodor_plib_funcs[j].sym);

                    exit(-EINVAL);
                  }
                  found = true;
                  break;
                }
              }

              if (!found)
                printf("libhodor: who's %s?, we'll ignore it.\n", str);
            }
            unsigned len = strlen(str);
            if (!len) break;
            i += len + 1;
          }
        }

        /* move pointer to the next entry */
        dyn++;
      }
    }
  }
  return 0;
}

int hodor_init(void) {
  hodor_fd = open("/dev/hodor", O_RDWR);
  if (hodor_fd <= 0) {
    printf(
        "libhodor: failed to open Hodor device. Is the module (kern/hodor.ko) "
        "loaded?\n");
    return -errno;
  }

  dl_iterate_phdr(elf_symtab_callback, NULL);

  dune_procmap_iterate(&__setup_mappings_cb);

  /*
   * For now, let's hard-code the PKEY = 1 for the (single) protected library
   * that we support.
   */
  pkey = pkey_alloc(0, PKEY_DISABLE_ACCESS);
  assert(pkey == 1);

  return ioctl(hodor_fd, HODOR_CONFIG, &config);
}

int hodor_enter() {
  int ret = 0;

  if (hodor_fd <= 0) {
    printf("libhodor: call hodor_init() before hodor_enter().\n");
    return -EINVAL;
  }

  ret = ioctl(hodor_fd, HODOR_ENTER);
  if (ret) {
    printf("libhodor: fail to enter.\n");
    goto out;
  }

  TLSP->stack =
      (unsigned long)mmap(NULL, PROTECTED_STACK_SIZE, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (!TLSP->stack) {
    printf("libhodor: failed allocating protected stack pages.\n");
    ret = -ENOMEM;
  }

  // point to the bottom of stack
  TLSP->stack += PROTECTED_STACK_SIZE;
  TLSU->stack = 0;

  /*
   * We will never need to write to kernel TLS pointed to by *HODOR_REG. While
   * not yet implemented, any modification to the protections and/or mapping new
   * executable pages will be intercepted by Hodor kernel module and vetted
   * (see. hodor_deny_signal() for how signals are vetted.)
   */
  mprotect(*HODOR_REG, PAGE_SIZE, PROT_READ | PROT_WRITE); /* BUG */
  pkey_mprotect(TLSP->stack - PROTECTED_STACK_SIZE, PROTECTED_STACK_SIZE,
                PROT_READ | PROT_WRITE, pkey);
  pkey_mprotect(TLSP, PAGE_SIZE, PROT_READ | PROT_WRITE, pkey);

out:
  return ret;
}
