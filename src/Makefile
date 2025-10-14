CC = gcc
FLEX = flex
BISON = bison
CFLAGS = -Wall
LDFLAGS = -lfl

all: comp

parser.tab.c parser.tab.h: parser.y
	$(BISON) -d parser.y

lex.yy.c: lexer.l parser.tab.h
	$(FLEX) lexer.l

comp: parser.tab.c lex.yy.c
	$(CC) -DUSE_PARSER -o comp parser.tab.c lex.yy.c $(LDFLAGS) $(CFLAGS)

run: comp
	./comp ../exemplos/program.src ../exemplos/out.asm

clean:
	rm -f parser.tab.c parser.tab.h lex.yy.c comp ../exemplos/out.asm