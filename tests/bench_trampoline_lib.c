#include <errno.h>
#include <stdio.h>
#include <strings.h>
#include <sys/mman.h>

#include <hodor-plib.h>

int bench_trampoline_init(void) { return 0; }
HODOR_INIT_FUNC(bench_trampoline_init);

HODOR_FUNC_ATTR void bench_trampoline(void) { return; }
HODOR_FUNC_EXPORT(bench_trampoline, 0);

void bench_notrampoline(void) { return; }
