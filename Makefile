.PHONY: test clean mem

TEST_SRC=$(wildcard utest/*.c)
TEST_OBJ=${TEST_SRC:.c=.o}

CFLAGS=-g -std=c11 -Wall -Wextra -Wno-missing-braces -Wno-unused-function -Iutest -I.
%.o: utest/%.c
	$(CC) $(CFLAGS) -c $<

all: demo
	$(CC) $(CFLAGS) -o demo demo.c
	-@./demo


test: ${TEST_OBJ}
	gcc -o $@ $^
	-@./test -v | ./greenest

mem: test
	-@valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ./test -v | ./greenest


clean:
	-@rm ./*.o ./test ./a.out ./demo ${TEST_OBJ} 2> /dev/null ||true
