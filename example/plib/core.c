#include <hodor-plib.h>

#include "plib.h"

/*
 * Hodor will call this function at load time (HODOR_INIT_FUNC instructs us to
 * do that). Any initialization (e.g., allocate private heap, etc.) should
 * happen here. Note that Hodor doesn't prevent the attacker from calling the
 * initialization function again.
 * Currently everything is hard-coded and protected library's pkey is 1. Use
 * pkey_mprotect() with pkey = 1 to change all private pages to only accessible
 * by code that goes through trampoline.
 */
int plib_init(void) { return 0; }
HODOR_INIT_FUNC(plib_init);

/*
 * Hodor needs space at the beginning of the function to insert a
 * trampoline. HODOR_FUNC_ATTR grantees it will have enough space. Also,
 * HODOR_FUNC_EXPORT tells Hodor loader to only call this function through
 * trampoline, plus the number of arguments that it takes. An attacker may still
 * call this function directly, but we guarantee that it would still be running
 * in the original domain.
 * NOTE: HODOR_FUNC_EXPORTed function must not be called from inside the
 * protected library (or your application will get a SEGSIGV). If you are
 * porting a library, wrap existing functions and HODOR_FUNC_EXPORT the
 * wrappers.
 */

HODOR_FUNC_ATTR int plib_sum(int a, int b) { return a + b; }
HODOR_FUNC_EXPORT(plib_sum, 2);

/*
 * Hodor will not insert a trampoline for plib_mul since it is not
 * HODOR_FUNC_EXPORTed. You would want to avoid paying the cost of trampolines
 * if your security model allows it (e.g., only reading a status without need
 * for making integrity-sensitive pages writable.)
 */
int plib_mul(int a, int b) { return a * b; }
