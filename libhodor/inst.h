#pragma once

#include "hodor.h"

static inline void x86_inst_retq(char *stream, unsigned long *idx) {
  /*
   * retq                         c3
   */
  stream[*idx + 0] = '\xc3';
  *idx += 1;
}

static inline void x86_inst_movabs_rax(char *stream, unsigned long *idx,
                                       unsigned long val) {
  /*
   * movabs $val, %rax           48 b8 xx xx xx xx xx xx xx xx
   */
  stream[*idx + 0] = '\x48';
  stream[*idx + 1] = '\xb8';
  stream[*idx + 2] = (char)(val >> 0) & 0xff;
  stream[*idx + 3] = (char)(val >> 8) & 0xff;
  stream[*idx + 4] = (char)(val >> 16) & 0xff;
  stream[*idx + 5] = (char)(val >> 24) & 0xff;
  stream[*idx + 6] = (char)(val >> 32) & 0xff;
  stream[*idx + 7] = (char)(val >> 40) & 0xff;
  stream[*idx + 8] = (char)(val >> 48) & 0xff;
  stream[*idx + 9] = (char)(val >> 56) & 0xff;
  *idx += 10;
}

static inline void x86_inst_movabs_r10(char *stream, unsigned long *idx,
                                       unsigned long val) {
  /*
   * movabs $val, %10           49 ba xx xx xx xx xx xx xx xx
   */
  stream[*idx + 0] = '\x49';
  stream[*idx + 1] = '\xba';
  stream[*idx + 2] = (char)(val >> 0) & 0xff;
  stream[*idx + 3] = (char)(val >> 8) & 0xff;
  stream[*idx + 4] = (char)(val >> 16) & 0xff;
  stream[*idx + 5] = (char)(val >> 24) & 0xff;
  stream[*idx + 6] = (char)(val >> 32) & 0xff;
  stream[*idx + 7] = (char)(val >> 40) & 0xff;
  stream[*idx + 8] = (char)(val >> 48) & 0xff;
  stream[*idx + 9] = (char)(val >> 56) & 0xff;
  *idx += 10;
}

static inline void x86_inst_jpmq_rax(char *stream, unsigned long *idx) {
  /*
   * jmpq   *%rax                 ff e0
   */
  stream[*idx + 0] = '\xff';
  stream[*idx + 1] = '\xe0';
  *idx += 2;
}

static inline void x86_inst_callq_rax(char *stream, unsigned long *idx) {
  /*
   * callq  %rax                  ff d0
   */
  stream[*idx + 0] = '\xff';
  stream[*idx + 1] = '\xd0';
  *idx += 2;
}

static inline void x86_inst_jmpq_rel32(char *stream, unsigned long *idx,
                                       unsigned long target) {
  /*
   * jmpq   1b                    e9 xx xx xx xx
   */
  unsigned long dis = (unsigned long)&stream[*idx + 5] - target;
  stream[*idx + 0] = '\xe9';
  stream[*idx + 1] = (char)((0x100000000L - dis) >> 0) & 0xff;
  stream[*idx + 2] = (char)((0x100000000L - dis) >> 8) & 0xff;
  stream[*idx + 3] = (char)((0x100000000L - dis) >> 16) & 0xff;
  stream[*idx + 4] = (char)((0x100000000L - dis) >> 24) & 0xff;
  *idx += 5;
}

static inline void x86_inst_rdpkru(char *stream, unsigned long *idx) {
  /*
   * rdpkru                       0f 01 ee
   */
  stream[*idx + 0] = '\x0f';
  stream[*idx + 1] = '\x01';
  stream[*idx + 2] = '\xee';
  *idx += 3;
}

/*
 * We have to prevent this from being inlined, or we will waste a breakpoint
 * inspecting it.
 */
static __attribute__((noinline)) void x86_inst_wrpkru(char *stream,
                                                      unsigned long *idx) {
  /*
   * wrpkru                       0f 01 ef
   */
  stream[*idx + 0] = '\x0f';
  stream[*idx + 1] = '\x01';
  stream[*idx + 2] = '\xef';
  *idx += 3;
}

static inline void x86_inst_push_rax(char *stream, unsigned long *idx) {
  /*
   * push %rax                       50
   */
  stream[*idx + 0] = '\x50';
  *idx += 1;
}

static inline void x86_inst_pop_rax(char *stream, unsigned long *idx) {
  /*
   * push %rax                       58
   */
  stream[*idx + 0] = '\x58';
  *idx += 1;
}

static inline void x86_inst_push_rbx(char *stream, unsigned long *idx) {
  /*
   * push %rbx                       53
   */
  stream[*idx + 0] = '\x53';
  *idx += 1;
}

static inline void x86_inst_pop_rbx(char *stream, unsigned long *idx) {
  /*
   * push %rbx                       5b
   */
  stream[*idx + 0] = '\x5b';
  *idx += 1;
}

static inline void x86_inst_push_rcx(char *stream, unsigned long *idx) {
  /*
   * push %rcx                       51
   */
  stream[*idx + 0] = '\x51';
  *idx += 1;
}

static inline void x86_inst_pop_rcx(char *stream, unsigned long *idx) {
  /*
   * push %rcx                       59
   */
  stream[*idx + 0] = '\x59';
  *idx += 1;
}

static inline void x86_inst_push_rdx(char *stream, unsigned long *idx) {
  /*
   * push %rdx                       52
   */
  stream[*idx + 0] = '\x52';
  *idx += 1;
}

static inline void x86_inst_pop_rdx(char *stream, unsigned long *idx) {
  /*
   * push %rdx                       5a
   */
  stream[*idx + 0] = '\x5a';
  *idx += 1;
}

static inline void x86_inst_xor_ecx_ecx(char *stream, unsigned long *idx) {
  /*
   * xor %ecx, %ecx                  31 c9
   */
  stream[*idx + 0] = '\x31';
  stream[*idx + 1] = '\xc9';
  *idx += 2;
}

static inline void x86_inst_xor_edx_edx(char *stream, unsigned long *idx) {
  /*
   * xor %edx, %edx                  31 d2
   */
  stream[*idx + 0] = '\x31';
  stream[*idx + 1] = '\xd2';
  *idx += 2;
}

static inline void x86_inst_mov_rsp_rax(char *stream, unsigned long *idx) {
  /*
   * mov %rsp, %rax                  48 89 e0
   */
  stream[*idx + 0] = '\x48';
  stream[*idx + 1] = '\x89';
  stream[*idx + 2] = '\xe0';
  *idx += 3;
}

static inline void x86_inst_mov_rax_atrax(char *stream, unsigned long *idx) {
  /*
   * mov %rax, (%rax)                 48 89 00
   */
  stream[*idx + 0] = '\x48';
  stream[*idx + 1] = '\x89';
  stream[*idx + 2] = '\x00';
  *idx += 3;
}

static inline void x86_inst_mov_atrax_rax(char *stream, unsigned long *idx) {
  /*
   * mov (%rax), %rax                 48 8b 00
   */
  stream[*idx + 0] = '\x48';
  stream[*idx + 1] = '\x8b';
  stream[*idx + 2] = '\x00';
  *idx += 3;
}

static inline void x86_inst_mov_atrax_rax_off8(char *stream, unsigned long *idx,
                                               unsigned short off) {
  /*
   * mov $xx(%rax), %rax                 48 8b 40 xx
   */
  stream[*idx + 0] = '\x48';
  stream[*idx + 1] = '\x8b';
  stream[*idx + 2] = '\x40';
  stream[*idx + 3] = (char)(off & 0xff);
  *idx += 4;
}

static inline void x86_inst_mov_atr10_r10(char *stream, unsigned long *idx) {
  /*
   * mov (%r10), %r10                 4d 8b 12
   */
  stream[*idx + 0] = '\x4d';
  stream[*idx + 1] = '\x8b';
  stream[*idx + 2] = '\x12';
  *idx += 3;
}

static inline void x86_inst_add_rax_8(char *stream, unsigned long *idx) {
  /*
   * add $xx, %rax                 48 83 c0 08
   */
  stream[*idx + 0] = '\x48';
  stream[*idx + 1] = '\x83';
  stream[*idx + 2] = '\xc0';
  stream[*idx + 3] = '\x08';
  *idx += 4;
}

static inline void x86_inst_mov_rsp_atrax(char *stream, unsigned long *idx) {
  /*
   * mov %rsp, (%rax)                 48 89 20
   */
  stream[*idx + 0] = '\x48';
  stream[*idx + 1] = '\x89';
  stream[*idx + 2] = '\x20';
  *idx += 3;
}

static inline void x86_inst_mov_atrax_rsp(char *stream, unsigned long *idx) {
  /*
   * mov (%rax), %rsp                 48 8b 20
   */
  stream[*idx + 0] = '\x48';
  stream[*idx + 1] = '\x8b';
  stream[*idx + 2] = '\x20';
  *idx += 3;
}

static inline void x86_inst_mov_atr10_rsp(char *stream, unsigned long *idx) {
  /*
   * mov (%r10), %rsp                 49 8b 22
   */
  stream[*idx + 0] = '\x49';
  stream[*idx + 1] = '\x8b';
  stream[*idx + 2] = '\x22';
  *idx += 3;
}

static inline void x86_inst_mov_eax(char *stream, unsigned long *idx,
                                    unsigned int val) {
  /*
   * mov $val, %eax                 b8 xx xx xx xx
   */
  stream[*idx + 0] = '\xb8';
  stream[*idx + 1] = (char)(val >> 0) & 0xff;
  stream[*idx + 2] = (char)(val >> 8) & 0xff;
  stream[*idx + 3] = (char)(val >> 16) & 0xff;
  stream[*idx + 4] = (char)(val >> 24) & 0xff;
  *idx += 5;
}

static inline void x86_inst_cmp_eax(char *stream, unsigned long *idx,
                                    unsigned long val) {
  /*
   * cmp $val, %eax                 3d xx xx xx xx
   */
  stream[*idx + 0] = '\x3d';
  stream[*idx + 1] = (char)(val >> 0) & 0xff;
  stream[*idx + 2] = (char)(val >> 8) & 0xff;
  stream[*idx + 3] = (char)(val >> 16) & 0xff;
  stream[*idx + 4] = (char)(val >> 24) & 0xff;
  *idx += 5;
}

static inline void x86_inst_jne_rel8(char *stream, unsigned long *idx,
                                     unsigned long target) {
  /*
   * jne   1b                    75 xx
   */
  unsigned long dis = (unsigned long)&stream[*idx + 2] - target;
  stream[*idx + 0] = '\x75';
  stream[*idx + 1] = (char)(0x100L - dis) & 0xff;
  *idx += 2;
}
