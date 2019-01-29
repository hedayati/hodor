#include <errno.h>
#include <hodor.h>
#include <stdio.h>
#include <strings.h>
#include <sys/mman.h>

extern void test_sig_delay_loop(void);

int main() {
  int ret = 0;

  ret = hodor_init();
  ret = hodor_enter();

  test_sig_delay_loop();

  return 0;
}
