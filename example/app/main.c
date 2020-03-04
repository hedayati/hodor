#include <assert.h>
#include <errno.h>
#include <hodor.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <plib.h>

#define N_THREAD_COUNT 128

void *thread_func(void *arg) {
  /*
   * The very first thing each thread does must be calling hodor_enter().
   * Ideally we want it to be a part of pthread (or whatever thread library
   * being used), but for now just manually call it.
   * hodor_enter() among other things create TLS pages and domain status page.
   */
  assert(hodor_enter() == 0);

  printf("hello world from child %d: %d\n", (int)arg, plib_sum(7, 8));
}

int main(int argc, char const *argv[]) {
  int ret = 0;
  pthread_t thread[N_THREAD_COUNT];

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

  for (int i = 0; i < N_THREAD_COUNT; ++i) {
    if (pthread_create(&thread[i], NULL, thread_func, (void *)i)) {
      exit(-EINVAL);
    }
  }

  printf("hello world from parent: %d\n", plib_sum(6, 7));

  for (int i = 0; i < N_THREAD_COUNT; ++i) {
    if (pthread_join(thread[i], NULL))
      exit(-EINVAL);
  }

  return 0;
}