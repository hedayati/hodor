CC	= gcc
CFLAGS	= -Wall -g -MD -O3 -I../plib -I../../libhodor

all: app

app: main.o
	$(CC) $(CFLAGS) $(<) -o app -L../plib -lplib -L../../libhodor -lhodor -ldl -lpthread

clean:
	rm -f *.o *.d app

-include *.d