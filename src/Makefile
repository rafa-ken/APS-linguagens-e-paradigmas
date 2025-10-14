CC = gcc
FLEX = flex
BISON = bison
CFLAGS = -Wall
LDFLAGS = -lfl

ALL_TESTS_DIR = ../tests/out

all: comp

parser.tab.c parser.tab.h: parser.y
	$(BISON) -d parser.y

lex.yy.c: lexer.l parser.tab.h
	$(FLEX) lexer.l

comp: parser.tab.c lex.yy.c
	$(CC) -DUSE_PARSER -o comp parser.tab.c lex.yy.c $(LDFLAGS) $(CFLAGS)

run: comp
	./comp ../exemplos/program.src ../exemplos/out.asm

test: comp
	mkdir -p $(ALL_TESTS_DIR)
	@echo "Running tests..."
	@for f in ../tests/*.src; do \
	  base=$$(basename $$f .src); \
	  echo " - $$base"; \
	  ./comp $$f $(ALL_TESTS_DIR)/$${base}.asm 2> $(ALL_TESTS_DIR)/$${base}.err || true; \
	done
	@echo "Tests finished. Outputs in $(ALL_TESTS_DIR)"

clean:
	rm -f parser.tab.c parser.tab.h lex.yy.c comp ../exemplos/out.asm
	rm -rf $(ALL_TESTS_DIR)

.PHONY: all run test clean
