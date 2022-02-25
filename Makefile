.PHONY: test clean

%.o: utest/%.c
	gcc -std=c11 -Wall -Wextra -Iutest -I. -c $<

test: misc.o scalar.o shared.o array.o test.o
	gcc -o $@ $^
	-@./test

clean:
	-@rm ./*.o ./test ./a.out
