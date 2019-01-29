#include <errno.h>
#include <stdio.h>
#include <strings.h>
#include <sys/mman.h>

#include <hodor-plib.h>

int test_sig_delay_init(void) { return 0; }
HODOR_INIT_FUNC(test_sig_delay_init);

int test_sig_delay_loop(void) {
  HODOR_FUNC_PROLOGUE;
  while (1)
    ;
}
HODOR_FUNC_EXPORT(test_sig_delay_loop, 0);
