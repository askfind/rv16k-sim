main: main.c elf_parser.c bitpat.c log.c inst.c ternary.c
	gcc -o $@ $^
clean:
	rm main
test:
	./test.sh

test_trit:
	./test_trit.sh

.PHONY: clean test
