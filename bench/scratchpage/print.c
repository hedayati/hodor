#include <stdio.h>

#define HODOR_SCRATCH_AREA	((unsigned long *)0xffffffffff578000)
#define HODOR_REG0			(HODOR_SCRATCH_AREA + sizeof(unsigned long)) 

int main() {
	unsigned long *cpu = HODOR_SCRATCH_AREA;
	unsigned long *reg0 = HODOR_REG0;
	printf("%ld reg0: %ld\n", *cpu, (*reg0)++);
	return 0;
}

