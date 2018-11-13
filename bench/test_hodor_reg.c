#include <stdio.h>
#include <unistd.h>

#include <hodor.h>

int main() {
  unsigned long *cpu = HODOR_SCRATCH_AREA;
  unsigned long *reg = HODOR_REG;
  while (1) {
    printf("%ld reg0: %ld\n", *cpu, (*reg)++);
    usleep(100000);
  }
  return 0;
}
