CC := gcc
LEX_SRC := scanner.l
YACC_SRC := parser.y
LEX := flex
YACC := bison

all: squash

squash: job.o
	$(CC) -o squash job.o memory.o lexer.o parser.o absyn.o

job.o: absyn.o
	$(CC) -c -o $@ $*.c

absyn.o: absyn.c
	$(CC) -c -o $@ $^

absyn.c absyn.h:
	asdl -d absyn.h -o absyn.c squash.asdl

memory.o: lexer.o
	$(CC) -c -o $@ $*.c

lexer.o: lex.yy.c parser.o lexer.h
	$(CC) -c -o $@ lex.yy.c

parser.o: parser.tab.c
	$(CC) -c -o $@ parser.tab.c

lex.yy.c: parser.tab.c
	$(LEX) --header-file=lexer.h $@

parser.tab.c: $(YACC_SRC)
	$(YACC) -d $^


.PHONY clean:
	rm -f lex.yy.c parser.tab.c parser.tab.h parser.o memory.o job.o lexer.o absyn.o absyn.c absyn.h squash
