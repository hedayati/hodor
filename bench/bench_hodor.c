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

struct hodor_tls *tls = NULL;

struct hodor_tls utls;

int main() {
  int ret = 0;

  ret = hodor_init();
  ret = hodor_enter(&utls);

  tls = (struct hodor_tls *)*HODOR_REG;

  printf("%lx\n", tls->config);

  wrpkru(rdpkru());

  void *page = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (!page) {
    printf("hodor: mmap() failed.\n");
    return -errno;
  }

  bzero(page, PAGE_SIZE);

  bzero(page, PAGE_SIZE);

  munmap(page, PAGE_SIZE);

  wrpkru(rdpkru() | 0xfffffff0);

  return 0;
}
