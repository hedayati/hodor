#include <assert.h>
#include <errno.h>
#include <hodor.h>
#include <stdio.h>
#include <strings.h>
#include <sys/mman.h>

static inline unsigned int rdpkru(void) {
  unsigned int eax, edx;
  unsigned int ecx = 0;
  unsigned int pkru;

  asm volatile(".byte 0x0f,0x01,0xee\n\t" : "=a"(eax), "=d"(edx) : "c"(ecx));
  pkru = eax;
  return pkru;
}

static inline void wrpkru(unsigned int pkru) {
  unsigned int eax = pkru;
  unsigned int ecx = 0;
  unsigned int edx = 0;

  asm volatile(".byte 0x0f,0x01,0xef\n\t" : : "a"(eax), "c"(ecx), "d"(edx));
}

int main() {
  int ret = 0;

  ret = hodor_init();
  assert(ret == 0);
  ret = hodor_enter();
  assert(ret == 0);

  printf("Segmentation fault is expected (look at dmesg).\n");

  wrpkru(rdpkru());

  wrpkru(rdpkru() | 0xfffffff0);

  return 0;
}
