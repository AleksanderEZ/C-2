# make F=n.x	[genera] y ejecuta sobre n.x; via stdin: make<n.x

all: c2 $(F)
	./c2 $(F)

clean:
	rm -f lex.yy.c parser.tab.* parser.output c2

c2: parser.tab.c lex.yy.c
	gcc -g -o c2 symbol_table.c parser.tab.c lex.yy.c

lex.yy.c: scanner.l parser.tab.h
	flex scanner.l 

parser.tab.c: parser.y symbol_table.c symbol_table.h
	bison -dvt parser.y
