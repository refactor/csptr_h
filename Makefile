.PHONY: test clean

%.o: utest/%.c
	gcc -g -O0 -std=c11 -Wall -Wextra -Iutest -I. -c $<

test: misc.o scalar.o shared.o array.o test.o flexarr.o
	gcc -g -O0 -o $@ $^
	-@./test

clean:
	-@rm ./*.o ./test ./a.out
