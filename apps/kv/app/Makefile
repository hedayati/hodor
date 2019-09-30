CC = gcc
CXXFLAGS = -std=c++17 -Wall -g -MD -O3 -I../libkv -I../../../libhodor

all: app

app: main.o
	$(CC) $(CFLAGS) $(<) -o app -L../libkv -lkv -L../../../libhodor -lhodor -ldl -lpthread -lstdc++ -lboost_container

clean:
	rm -f *.o *.d app

-include *.d