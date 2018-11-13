CC	= gcc
CFLAGS	= -Wall -g -MD -O3 -I../libhodor

all: test_hodor_reg bench_hodor

bench_hodor: bench_hodor.o
	$(CC) $(CFLAGS) $(<) -o bench_hodor -L../libhodor -lhodor

test_hodor_reg: test_hodor_reg.o
	$(CC) $(CFLAGS) $(<) -o test_hodor_reg -L../libhodor -lhodor

clean:
	rm -f *.o *.d test_hodor_reg bench_hodor

-include *.d