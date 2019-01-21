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
struct hodor_config config;

char *hodor_plib_path = NULL;
char *hodor_plib_init_func = NULL;
char *hodor_plib_funcs[1024];
unsigned hodor_plib_funcs_count = 0;
typedef int (*hodor_plib_init_t)(void);

static void hodor_insert_trampoline(char *func_name, char *func_text,
                                    char *tramp_text,
                                    unsigned long *tramp_idx) {
  unsigned long i;
  unsigned long func_text_idx = 0;

  for (i = 0; i < 32; ++i)
    if (func_text[i] != '\x90') {
      printf("libhodor: PLIB_FUNC_PROLOGUE missing from %s.\n", func_name);
      exit(-EINVAL);
    }

  /*
   * movabs $tramp, %rax    48 b8 xx xx xx xx xx xx xx xx
   * jmpq   *%rax           ff e0
   */
  x86_inst_movabs_rax(func_text, &func_text_idx,
                      (unsigned long)&tramp_text[*tramp_idx]);
  x86_inst_jpmq_rax(func_text, &func_text_idx);

  /*
   * 1:
   * retq                         c3
   * ***********************************************************
   * flip protection
   * movabs $(text + 32), %rax    48 b8 xx xx xx xx xx xx xx xx
   * callq  %rax                  ff d0
   * flip protection again
   * jmpq   1b                    e9 xx xx xx xx
   * ***********************************************************
   */
  x86_inst_push_rcx(tramp_text, tramp_idx);
  x86_inst_push_rdx(tramp_text, tramp_idx);
  x86_inst_xor_ecx_ecx(tramp_text, tramp_idx);
  x86_inst_xor_edx_edx(tramp_text, tramp_idx);
  unsigned long lbl0 = (unsigned long)&tramp_text[*tramp_idx];
  x86_inst_rdpkru(tramp_text, tramp_idx);
  x86_inst_wrpkru(tramp_text, tramp_idx);
  x86_inst_cmp_eax(tramp_text, tramp_idx, 0x55555554);
  x86_inst_jne_rel8(tramp_text, tramp_idx, lbl0);
  x86_inst_pop_rdx(tramp_text, tramp_idx);
  x86_inst_pop_rcx(tramp_text, tramp_idx);

  x86_inst_movabs_rax(tramp_text, tramp_idx, (unsigned long)func_text + 32);
  x86_inst_callq_rax(tramp_text, tramp_idx);

  x86_inst_push_rax(tramp_text, tramp_idx);
  x86_inst_push_rcx(tramp_text, tramp_idx);
  x86_inst_push_rdx(tramp_text, tramp_idx);
  x86_inst_xor_ecx_ecx(tramp_text, tramp_idx);
  x86_inst_xor_edx_edx(tramp_text, tramp_idx);
  unsigned long lbl1 = (unsigned long)&tramp_text[*tramp_idx];
  x86_inst_rdpkru(tramp_text, tramp_idx);
  x86_inst_wrpkru(tramp_text, tramp_idx);
  x86_inst_cmp_eax(tramp_text, tramp_idx, 0x55555554);
  x86_inst_jne_rel8(tramp_text, tramp_idx, lbl1);
  x86_inst_pop_rdx(tramp_text, tramp_idx);
  x86_inst_pop_rcx(tramp_text, tramp_idx);
  x86_inst_pop_rax(tramp_text, tramp_idx);

  x86_inst_jmpq_rel32(tramp_text, tramp_idx, (unsigned long)tramp_text);
}

static void __setup_mappings_cb(const struct dune_procmap_entry *ent) {
  if (ent->x) {
    unsigned long i;
    char *text;
    unsigned index = config.region_count;
    bool is_plib = (strcmp(ent->path, hodor_plib_path) == 0);
    void *plib_handle = NULL;
    char *tramp_text = NULL;
    unsigned long tramp_idx = 0;

    if (is_plib) {
      plib_handle = dlopen(ent->path, RTLD_LAZY);
      if (!plib_handle) {
        printf("libhodor: failed to open handle for %s\n", ent->path);
        exit(-EINVAL);
      }

      tramp_text = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
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

    if (!is_plib) {
      text = (char *)ent->begin;
      for (i = 0; i < config.region_len[index] - 3; ++i) {
        if (text[i] == '\x0F' && text[i + 1] == '\x01' &&
            text[i + 2] == '\xEF') {
          config.inspect_begin_va[config.inspect_count++] =
              (unsigned long)&text[i];
        }
      }
    } else {
      mprotect((void *)ent->begin, ent->end - ent->begin,
               PROT_READ | PROT_WRITE);

      for (i = 0; i < hodor_plib_funcs_count; ++i) {
        char *func_text = dlsym(plib_handle, hodor_plib_funcs[i]);
        if (!func_text) {
          printf("libhodor: failed finding function %s from %s\n",
                 hodor_plib_funcs[i], ent->path);
          exit(-EINVAL);
        }

        hodor_insert_trampoline(hodor_plib_funcs[i], func_text, tramp_text,
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

static int callback(struct dl_phdr_info *info, size_t size, void *data) {
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
              hodor_plib_funcs[hodor_plib_funcs_count] = malloc(strlen(str));
              strcpy(hodor_plib_funcs[hodor_plib_funcs_count], str + 13);
              hodor_plib_funcs_count++;
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
    printf("libhodor: failed to open Hodor device.\n");
    return -errno;
  }

  dl_iterate_phdr(callback, NULL);

  dune_procmap_iterate(&__setup_mappings_cb);

  return ioctl(hodor_fd, HODOR_CONFIG, &config);
}

int hodor_enter() {
  if (hodor_fd <= 0) {
    printf("libhodor: call hodor_init() before hodor_enter.\n");
    return -EINVAL;
  }

  return ioctl(hodor_fd, HODOR_ENTER);
}
