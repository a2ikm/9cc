CFLAGS=-std=c11 -g -static

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

TEST_SRCS=$(wildcard test/*.c)
TESTS=$(TEST_SRCS:.c=.exe)

9cc: $(OBJS)
	$(CC) $(CFLAGS) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test/%.exe: 9cc test/%.c
	$(CC) $(CFLAGS) -o- -E -P -C test/$*.c | ./9cc - > test/$*.s
	$(CC) $(CFLAGS) -o $@ test/$*.s -xc test/common

test: $(TESTS)
	for i in $^; do echo $$i; ./$$i || exit 1; echo; done

clean:
	rm -f 9cc *.o *~ tmp* test/*.o test/*.s test/*.exe

.PHONY: test clean
