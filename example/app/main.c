#include <assert.h>
#include <errno.h>
#include <hodor.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <plib.h>

void *thread_func(void *ptr) {
  /*
   * The very first thing each thread does must be calling hodor_enter().
   * Ideally we want it to be a part of pthread (or whatever thread library
   * being used), but for now just manually call it.
   * hodor_enter() among other things create TLS pages and domain status page.
   */
  assert(hodor_enter() == 0);

  printf("hello world from child: %d\n", plib_sum(7, 8));

  return NULL;
}

int main(int argc, char const *argv[]) {
  int ret = 0;
  pthread_t thread;

  /*
   * Ideally a loader should take care of following two lines, but until
   * somebody writes a functioning loader we have to do it manually.
   * hodor_init() configures the inserts trampolines and illegal instruction
   * inspections.
   * hodor_enter() create status pages (aka domain status pages) to save/restore
   * states when transitioning between domains.
   * Both require Hodor-PKU kernel and kern/hodor.ko should be loaded.
   */
  ret = hodor_init();
  assert(ret == 0);
  ret = hodor_enter();
  assert(ret == 0);

  if (pthread_create(&thread, NULL, thread_func, NULL)) {
    exit(-EINVAL);
  }

  printf("hello world from parent: %d\n", plib_sum(6, 7));

  if (pthread_join(thread, NULL)) exit(-EINVAL);

  return 0;
}

