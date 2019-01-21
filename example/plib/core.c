#include <hodor-plib.h>

#include "plib.h"

/*
 * Hodor will call this function at load time (HODOR_INIT_FUNC instructs us to
 * do that). Any initialization (e.g., allocate private heap, etc.) should
 * happen here. Note that Hodor doesn't prevent the attacker from calling the
 * initialization function again.
 */
int plib_init(void) { return 0; }
HODOR_INIT_FUNC(plib_init);

/*
 * Hodor needs space at the beginning of the function to insert a
 * trampoline. HODOR_FUNC_PROLOGUE grantees it will have enough space. Also,
 * HODOR_FUNC_EXPORT tells Hodor loader to only call this function through
 * trampoline. An attacker may still call this function directly, but we
 * guarantee that it would still be running in the original domain.
 */

int plib_sum(int a, int b) {
  HODOR_FUNC_PROLOGUE;
  return a + b;
}
HODOR_FUNC_EXPORT(plib_sum);

/*
 * Hodor will not insert a trampoline for plib_mul since it is not *
 * HODOR_FUNC_EXPORTed. You would want to avoid paying the cost of trampolines
 * if your security model allows it (e.g., only reading a status without need
 * for making integrity-sensitive pages writable.)
 */
int plib_mul(int a, int b) { return a * b; }
