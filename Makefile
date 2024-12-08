CC := gcc
LEX_SRC := scanner.l
YACC_SRC := parser.y
LEX := flex
YACC := bison

all: squash

squash: job.o
	$(CC) -o squash job.o memory.o lexer.o parser.o

job.o: memory.o
	$(CC) -c -o $@ $*.c

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

