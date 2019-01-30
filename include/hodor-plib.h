#pragma once

#define NOPS(n) asm volatile(".fill %c0, 1, 0x90" ::"i"(n))

/*
 * Any HODOR_FUNC_EXPORTed function must use HODOR_FUNC_PROLOGUE at its
 * beginning to allow sufficient space for trampoline hooks. HODOR_FUNC_PROLOGUE
 * only inserts nop instructions so it does not affect the functionality of the
 * library even if it is loaded without Hodor.
 */
#define HODOR_FUNC_PROLOGUE NOPS(32)

/*
 * Hodor will call #sym function at load time. Any initialization (e.g.,
 * allocate private heap, etc.) should happen here.
 * NOTE: Hodor doesn't prevent this function from being called after
 * initialization. Do yourself a favor and take care of that if you need to.
 */
#define HODOR_INIT_FUNC(sym) \
  void __hodor_init_##sym(void) {}

/*
 * HODOR_FUNC_EXPORT tells Hodor loader to only call this function through
 * trampoline, plus the number of arguments that it takes. An attacker may still
 * call this function directly, but we guarantee that it would still be running
 * in the original domain.
 * NOTE: Any HODOR_FUNC_EXPORTed function must use HODOR_FUNC_PROLOGUE at its
 * beginning to allow sufficient space for trampoline hooks. HODOR_FUNC_PROLOGUE
 * only inserts `nop` instructions so it does not affect the functionality of
 * the library even if it is loaded without Hodor.
 */
#define HODOR_FUNC_EXPORT(sym, n)  \
  void __hodor_func_##sym(void){}; \
  void __hodor_narg_##sym##_##n(void) {}
