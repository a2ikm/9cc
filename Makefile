CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc
	./test.sh

clean:
	rm -f 9cc *.o *~ tmp*

one: 9cc
	./9cc "a = 42; a + 58;" > tmp.s
	gcc -o tmp tmp.s
	./tmp || true
	echo Done

.PHONY: test clean one
