CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -pedantic -g
COMMON  := order.o queue.o stack.o sim.o

.PHONY: all test memcheck sanitize clean

all: qs qs_tests

qs: main.o $(COMMON)
	$(CC) $(CFLAGS) $^ -o $@

qs_tests: tests.o $(COMMON)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c *.h
	$(CC) $(CFLAGS) -c $< -o $@

test: qs_tests
	./qs_tests

memcheck: qs qs_tests
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./qs > /dev/null
	valgrind --leak-check=full --error-exitcode=1 ./qs_tests > /dev/null

sanitize:
	$(CC) $(CFLAGS) -fsanitize=address,undefined main.c order.c queue.c stack.c sim.c -o qs_san
	./qs_san > /dev/null && echo "ASan/UBSan: clean run, no leaks reported"

clean:
	$(RM) *.o qs qs_tests qs_san
