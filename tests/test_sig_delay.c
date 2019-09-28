#include <assert.h>
#include <errno.h>
#include <hodor.h>
#include <stdio.h>
#include <strings.h>
#include <sys/mman.h>

extern void test_sig_delay_loop(void);

int main() {
  int ret = 0;

  ret = hodor_init();
  assert(ret == 0);
  ret = hodor_enter();
  assert(ret == 0);

  printf("Press Ctrl+C (then look at dmesg).\n");
  test_sig_delay_loop();

  return 0;
}
