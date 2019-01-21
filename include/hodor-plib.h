#pragma once

#define NOPS(n) asm volatile(".fill %c0, 1, 0x90" ::"i"(n))
#define HODOR_FUNC_PROLOGUE NOPS(32)

#define HODOR_INIT_FUNC(sym) \
  void __hodor_init_##sym(void) {}

#define HODOR_FUNC_EXPORT(sym) \
  void __hodor_func_##sym(void) {}
