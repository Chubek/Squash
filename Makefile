CC := gcc
LEX_SRC := scanner.l
YACC_SRC := parser.y
LEX := flex
YACC := bison

all: squash

squash: job.o memory.o absyn.o
	$(CC) $(DEBUG) -o $@ job.o memory.o parser.o lexer.o absyn.o

job.o: job.c absyn.h lexer.o parser.o
	$(CC) $(DEBUG) -c -o $@ $*.c

absyn.o: absyn.c
	$(CC) $(DEBUG) -c -o $@ $^

memory.o: memory.c lexer.h
	$(CC) $(DEBUG) -c -o $@ memory.c

lexer.o: lex.yy.c lexer.h parser.o
	$(CC) $(DEBUG) -c -o $@ lex.yy.c

parser.o: parser.tab.c parser.tab.h
	$(CC) $(DEBUG) -c -o $@ parser.tab.c

lex.yy.c lexer.h: $(LEX_SRC)
	$(LEX) --header-file=lexer.h $^

parser.tab.c parser.tab.h: $(YACC_SRC)
	$(YACC) -d $^

.PHONY: clean
clean:
	rm -f lex.yy.c parser.tab.c parser.tab.h parser.o memory.o job.o lexer.o absyn.o lexer.h squash
