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
	./9cc "int main() { int a; a = 42; return a; }" > tmp.s
	gcc -o tmp tmp.s
	./tmp
	echo Done

.PHONY: test clean one
