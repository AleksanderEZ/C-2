TESTFILE = src/PrintHelloWorld.c2
OBJECTFILE = Q/obj.q.c
IQ = Q/IQ.o

all: $(OBJECTFILE) Q/qmachine
	Q/qmachine $(OBJECTFILE)

debug: c2 Q/qmachine
	gdb -q c2 -ex 'run $(TESTFILE) $(OBJECTFILE)'
	Q/qmachine -g $(OBJECTFILE)

Q/qmachine: $(IQ) Q/Qlib.c Q/Qlib.h Q/Q.h
	gcc -no-pie -o Q/qmachine $(IQ) Q/Qlib.c

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
	bison -dvt parser.y