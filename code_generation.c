#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "code_generation.h"
#include "symbol_table.h"

FILE* obj;
int lineSizeLimit = 100;
int label = 0;
int atLabel = 0;
int statCodeCounter = 0;
int stackBase = 0x11fff;
int stackTop = 0x11fff;

void pushStack(int bytes) {
    stackTop = stackTop - (bytes - 1);
}

void newLabel() {
    fprintf(obj, "L %d:", label);
    label++;
    atLabel = 1;
}

void setObjFile(char* objPath) {
    obj = fopen(objPath, "w");
}

void qInit() {
    fprintf(obj, "#include \"Q.h\"\n\nBEGIN\n");
    newLabel();
}

void qEnd() {
    if (atLabel == 1) {
        fprintf(obj, "\tGT(__fin);\n");
    } else {
        fprintf(obj, "\t\tGT(__fin);\n");
    }
    fprintf(obj, "END");
}

void qLine(char* line) {
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
    char* line = malloc(sizeof(char) * lineSizeLimit);

    snprintf(line, sizeof(char) * lineSizeLimit, "STAT(%d)", statCodeCounter);
    qLine(line);

    pushStack(strlen(expression)+1);
    snprintf(line, sizeof(char) * lineSizeLimit, "STR(0x%x,\"%s\"); ", stackTop, expression);
    qLine(line);
    
    snprintf(line, sizeof(char) * lineSizeLimit, "CODE(%d)", statCodeCounter);
    qLine(line);

    statCodeCounter++;

    snprintf(line, sizeof(char) * lineSizeLimit, "R0=%d;", label);
    qLine(line);

    snprintf(line, sizeof(char) * lineSizeLimit, "R1=0x%x;", stackTop);
    qLine(line);

    qLine("GT(putf_);");
    free(line);
    newLabel();
}

void qPrintExplicitFormat(char* formatString, char* arguments) {

}

void qPrintImplicitFormat(char* identifier, char* arguments) {

}