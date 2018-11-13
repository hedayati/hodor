CC      = gcc
CFLAGS  = -Wall -g -O3 -MD
LDFLAGS = -static -L. -lhodor
OBJ     = hodor.o procmap.o

all: libhodor.a

libhodor.a: $(OBJ)
	$(AR) crD $(@) $(OBJ)

clean:
	rm -f *.o *.d libhodor.a

-include *.d

%.o: %.c
	$(CC) $(CFLAGS) $(EXTRA_FLAGS) -o $@ -c $<

