CC = gcc
CFLAGS = -fPIC -Wall -Wextra -O2 -I../../include
LDFLAGS = -shared 
LIB = libplib.so

SRCS = core.c
OBJS = $(SRCS:.c=.o)

.PHONY: all
all: ${LIB}

$(LIB): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

$(SRCS:.c=.d):%.d:%.c
	$(CC) $(CFLAGS) -MM $< >$@

include $(SRCS:.c=.d)

.PHONY: clean
clean:
	rm -rf ${LIB} ${OBJS} $(SRCS:.c=.d)
