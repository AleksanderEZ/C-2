#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "code_generation.h"
#include "symbol_table.h"

FILE* obj;
int lineSizeLimit = 100;
int label = 0;
int atLabel = 0;
int continueLabel;
int breakLabel;
int statCodeCounter = 0;
int stackBase = 0x11fff;
int stackTop = 0x11fff;
char* line;


void pushStack(int bytes) {
    stackTop = stackTop - (bytes - 1);
}

void advanceLabel() {
    label++;
}

void newLabel() {
    fprintf(obj, "L %d:", label);
    atLabel = 1;
}

void setObjFile(char* objPath) {
    obj = fopen(objPath, "w");
}

void qInit() {
    line = malloc(sizeof(char) * lineSizeLimit);
    fprintf(obj, "#include \"Q.h\"\n\nBEGIN\n");
    newLabel();
    advanceLabel();
}

void qEnd() {
    if (atLabel == 1) {
        fprintf(obj, "\tGT(__fin);\n");
    } else {
        fprintf(obj, "\t\tGT(__fin);\n");
    }
    fprintf(obj, "END");
    free(line);
}

void qLine() {
    if (atLabel == 1) {
        fprintf(obj, "\t%s\n", line);
        atLabel = 0;
        return;
    }
    fprintf(obj, "\t\t%s\n", line);
}

void qCallFunction(char* function, char* arguments) {

}

void qCallFunctionNoArgs(char* function) {

}

void qMalloc(char* expression) {

}

void qSizeOf(char* expression) {

}

void qPrint(char* expression) {
    snprintf(line, sizeof(char) * lineSizeLimit, "STAT(%d)", statCodeCounter);
    qLine();

    pushStack(strlen(expression)+1); //strlen does not count /0
    snprintf(line, sizeof(char) * lineSizeLimit, "STR(0x%x,\"%s\"); ", stackTop, expression);
    qLine();
    
    snprintf(line, sizeof(char) * lineSizeLimit, "CODE(%d)", statCodeCounter);
    qLine();

    statCodeCounter++;

    snprintf(line, sizeof(char) * lineSizeLimit, "R0=%d;", label);
    qLine();

    snprintf(line, sizeof(char) * lineSizeLimit, "R1=0x%x;", stackTop);
    qLine();

    qLine("GT(putf_);");
    newLabel();
    advanceLabel();
}

void qPrintExplicitFormat(char* formatString, char* arguments) {

}

void qPrintImplicitFormat(char* identifier, char* arguments) {

}

void qStartWhile() {
    continueLabel = label;
    newLabel();
    advanceLabel();
    breakLabel = label;
    advanceLabel();
}

void qWhileCondition() {
    snprintf(line, sizeof(char) * lineSizeLimit, "IF(!R1) GT(%d)", breakLabel);
    qLine();
}

void qInstruction(char* instruction) {
    snprintf(line, sizeof(char) * lineSizeLimit, instruction);
    qLine();
}

void qFinishWhile() {
    snprintf(line, sizeof(char) * lineSizeLimit, "GT(%d)", continueLabel);
    qLine();
    int auxLabel = label;
    label = breakLabel;
    newLabel();
    label = auxLabel;
}