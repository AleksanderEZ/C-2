TESTFILE = src/Sum2.c2
OBJECTFILE = obj.q.c
IQ = IQ.o

all: $(OBJECTFILE) qmachine
	./qmachine $(OBJECTFILE)
	rm -f IQ-cpp.q.c

debug: c2 qmachine
	gdb -q c2 -ex 'run $(TESTFILE) $(OBJECTFILE)'
	./qmachine -g $(OBJECTFILE)
	rm -f IQ-cpp.q.c

qmachine: $(IQ) Qlib.c Qlib.h Q.h
	gcc -no-pie -o qmachine $(IQ) Qlib.c

obj: $(OBJECTFILE)
	less $(OBJECTFILE)

$(OBJECTFILE): c2
	./c2 $(TESTFILE) $(OBJECTFILE)

clean:
	rm -f lex.yy.c parser.tab.* parser.output c2

c2: parser.tab.c lex.yy.c
	gcc -g -o c2 symbol_table.c code_generation.c parser.tab.c lex.yy.c

lex.yy.c: scanner.l parser.tab.h
	flex scanner.l 

parser.tab.c: parser.y symbol_table.c symbol_table.h code_generation.c code_generation.h
	bison -dvt parser.y -Wcounterexamples