#include <errno.h>
#include <hodor.h>
#include <stdio.h>
#include <strings.h>
#include <sys/mman.h>

int main() {
  int ret = 0;

  ret = hodor_init();
  ret = hodor_enter();

  while (1)
    ;

  return 0;
}
