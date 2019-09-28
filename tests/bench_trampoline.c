#include <assert.h>
#include <errno.h>
#include <hodor.h>
#include <stdio.h>
#include <strings.h>
#include <sys/mman.h>

#include <measure.h>

#define ITERATIONS 1000000

extern void bench_trampoline(void);
extern void bench_notrampoline(void);

int main() {
  int ret = 0;
  unsigned long i, start, end;

  ret = hodor_init();
  assert(ret == 0);
  ret = hodor_enter();
  assert(ret == 0);

  start = get_ticks_start();
  for (i = 0; i < ITERATIONS; ++i) {
    bench_notrampoline();
  }
  end = get_ticks_end();
  printf("bench_notrampoline\t=\t%llu cycles.\n",
         (long long unsigned int)(end - start) / ITERATIONS);

  start = get_ticks_start();
  for (i = 0; i < ITERATIONS; ++i) {
    bench_trampoline();
  }
  end = get_ticks_end();
  printf("bench_trampoline\t=\t%llu cycles.\n",
         (long long unsigned int)(end - start) / ITERATIONS);

  return 0;
}
