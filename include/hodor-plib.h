#pragma once

/*
 * Any HODOR_FUNC_EXPORTed function must use HODOR_FUNC_ATTR to allow sufficient
 * space for trampoline hooks. HODOR_FUNC_ATTR only inserts an 8-byte nop
 * instruction so it does not affect the functionality of the library even if it
 * is loaded without Hodor.
 */
#define HODOR_FUNC_ATTR             \
  __attribute__((ms_hook_prologue)) \
  __attribute__((aligned(8)))       \
  __attribute__((noinline))         \
  __attribute__((noclone))

#ifndef __cplusplus
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
 * NOTE: Any HODOR_FUNC_EXPORTed function must use HODOR_FUNC_ATTR to allow
 * sufficient space for trampoline hooks. HODOR_FUNC_ATTR only inserts an 8-byte
 * nop instruction so it does not affect the functionality of the library even
 * if it is loaded without Hodor.
 */
#define HODOR_FUNC_EXPORT(sym, n)  \
  void __hodor_func_##sym(void){}; \
  void __hodor_argc_##sym##_##n(void) {}
#else
#define HODOR_INIT_FUNC(sym)       \
  extern "C" {                     \
  void __hodor_init_##sym(void) {} \
  }

#define HODOR_FUNC_EXPORT(sym, n)        \
  extern "C" {                           \
  void __hodor_func_##sym(void){};       \
  void __hodor_argc_##sym##_##n(void) {} \
  }
#endif
