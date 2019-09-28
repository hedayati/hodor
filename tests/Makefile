CC	= gcc
CFLAGS	= -Wall -g -MD -O3 -I../libhodor -I../include

all: test_instr_inspection test_sig_delay bench_trampoline

test_instr_inspection: test_instr_inspection.o
	$(CC) $(CFLAGS) $(<) -o test_instr_inspection -L../libhodor -lhodor -ldl

test_sig_delay: test_sig_delay.o libtest_sig_delay
	$(CC) $(CFLAGS) $(<) -o test_sig_delay -L../libhodor -lhodor -ldl -L. -ltest_sig_delay

libtest_sig_delay:
	$(CC) -c -fPIC -Wall -Wextra -O2 -I../include test_sig_delay_lib.c -o test_sig_delay_lib.o
	$(CC) -shared test_sig_delay_lib.o -o libtest_sig_delay.so

bench_trampoline: bench_trampoline.o libbench_trampoline
	$(CC) $(CFLAGS) $(<) -o bench_trampoline -L../libhodor -lhodor -ldl -L. -lbench_trampoline

libbench_trampoline:
	$(CC) -c -fPIC -Wall -Wextra -O2 -I../include bench_trampoline_lib.c -o bench_trampoline_lib.o
	$(CC) -shared bench_trampoline_lib.o -o libbench_trampoline.so

clean:
	rm -f *.o *.d test_instr_inspection test_sig_delay libtest_sig_delay.so bench_trampoline libbench_trampoline.so

-include *.d