#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <linux/types.h>

#include "hodor.h"

int hodor_fd;
struct hodor_config config;

#define plib_name "ld-2.27.so"

static void __setup_mappings_cb(const struct dune_procmap_entry *ent) {
  if (ent->x) {
    unsigned long i;
    char *text;
    unsigned index = config.region_count;
    config.region_begin_va[index] = ent->begin;
    config.region_len[index] = ent->end - ent->begin;
    config.region_pkey[index] = 0;
    config.exit_tramp_va[index] = 0;

    text = (char *)ent->begin;
    for (i = 0; i < config.region_len[index] - 3; ++i) {
      if (text[i] == '\x0F' && text[i + 1] == '\x01' && text[i + 2] == '\xEF') {
        config.inspect_begin_va[config.inspect_count++] = &text[i];
      }
    }

    config.region_count++;
  }
}

int hodor_init(void) {
  hodor_fd = open("/dev/hodor", O_RDWR);
  if (hodor_fd <= 0) {
    printf("libhodor: failed to open Hodor device.\n");
    return -errno;
  }

  dune_procmap_iterate(&__setup_mappings_cb);

  return ioctl(hodor_fd, HODOR_CONFIG, &config);
}

int hodor_enter(struct hodor_tls *tls) {
  return ioctl(hodor_fd, HODOR_ENTER, tls);
}
