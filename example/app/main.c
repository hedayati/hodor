#include <hodor.h>
#include <stdio.h>

#include <plib.h>

int main(int argc, char const *argv[]) {
  int ret = 0;

  /*
   * Ideally a loader should take care of following two lines, but until
   * somebody writes a functioning loader we have to do it manually.
   * hodor_init() configures the inserts trampolines and illegal instruction
   * inspections.
   * hodor_enter() create status pages (aka domain status pages) to save/restore
   * states when transitioning between domains.
   * Both require Hodor-PKU kernel and kern/hodor.ko to be loaded.
   */
  ret = hodor_init();
  ret = hodor_enter();

  printf("hello world! %d\n", plib_sum(6, 7));

  return 0;
}