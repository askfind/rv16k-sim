main: main.c elf_parser.c bitpat.c log.c ternary.c inst.c inst_trit.c
	gcc -o $@ $^

run:  main
	./main

clean:
	rm main
test:
	./test.sh

test_trit:
	./test_trit.sh


.PHONY: clean test test_trit
