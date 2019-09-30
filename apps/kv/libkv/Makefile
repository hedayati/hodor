CC = g++
CPPFLAGS = -std=c++17 -fPIC -Wall -Wextra -O2 -I../../../include/ -I../../../libhodor
LDFLAGS = -shared /usr/lib64/libboost_container.a
LIB = libkv.so

SRCS = core.cc
OBJS = $(SRCS:.cc=.o)

.PHONY: all
all: ${LIB}

$(LIB): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

$(SRCS:.cc=.d):%.d:%.cc
	$(CC) $(CFLAGS) -MM $< >$@

include $(SRCS:.cc=.d)

.PHONY: clean
clean:
	rm -rf ${LIB} ${OBJS} $(SRCS:.cc=.d)
