CC	= gcc
CFLAGS	= -Wall -g -MD -O3 -I../libhodor

all: test_hodor_reg test_instr_inspection test_sig_delay

test_instr_inspection: test_instr_inspection.o
	$(CC) $(CFLAGS) $(<) -o test_instr_inspection -L../libhodor -lhodor

test_sig_delay: test_sig_delay.o
	$(CC) $(CFLAGS) $(<) -o test_sig_delay -L../libhodor -lhodor

test_hodor_reg: test_hodor_reg.o
	$(CC) $(CFLAGS) $(<) -o test_hodor_reg -L../libhodor -lhodor

clean:
	rm -f *.o *.d test_hodor_reg test_instr_inspection test_sig_delay

-include *.d