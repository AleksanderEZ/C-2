#include <stdio.h>
#include <stdlib.h>

#include "code_generation.h"
#include "symbol_table.h"

FILE* obj;
int label = 0;
int atLabel = 0;
int statCodeCounter = 0;

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
        fprintf(obj, "\nEND");
        return;
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
    // STAT
    // STR
    // CODE

    char* line = malloc(sizeof(char) * 100);
    snprintf(line, sizeof(char) * 100, "R0=%d;", label);
    qLine(line);

    int address = 0x11ff6; //example
    snprintf(line, sizeof(char) * 100, "R1=0x%x;", address);
    qLine(line);

    qLine("GT(putf_);");
    free(line);
    newLabel();
}

void qPrintExplicitFormat(char* formatString, char* arguments) {

}

void qPrintImplicitFormat(char* identifier, char* arguments) {

}