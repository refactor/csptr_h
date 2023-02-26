.PHONY: test clean

%.o: utest/%.c
	gcc -g -O0 -std=c11 -Wall -Wextra -Wno-missing-braces -Wno-unused-function -Iutest -I. -c $<

test: misc.o scalar.o shared.o array.o test.o array2.o
	gcc -g -O0 -o $@ $^
	-@./test

clean:
	-@rm ./*.o ./test ./a.out 2> /dev/null ||true
