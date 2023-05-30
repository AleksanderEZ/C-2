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
int* registers;

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

int qAssignRegister() {
    int reg;
    for (int i = 0; i < 7; i++) {
        if(registers[i] == 0) {
            reg = i;
            registers[i] = 1;
            break;
        }
    }
    return reg;
}

void qFreeRegister(int reg) {
    registers[reg] = 0;
}

void qFreeRegisters() {
    for (int i = 0; i < 7; i++) {
        registers[i] = 0;
    }
}

void qInit() {
    registers = malloc(sizeof(int) * 7);
    for (int i = 0; i < 7; i++) {
        registers[i] = 0;
    }
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
    free(registers);
}

void qInstruction(char* instruction) {
    snprintf(line, sizeof(char) * lineSizeLimit, "%s", instruction);
    qLine();
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

void qMalloc(int reg) {

}

void qSizeOf(char* expression) {

}

void qPrintReg(int reg) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R0=%d;", label);
    qLine();

    snprintf(line, sizeof(char) * lineSizeLimit, "R1=R%d;", reg);
    qLine();

    qInstruction("GT(putf_);");

    newLabel();
    advanceLabel();
}

void qPrintExplicit(char* expression) {
    snprintf(line, sizeof(char) * lineSizeLimit, "STAT(%d)", statCodeCounter);
    qLine();

    pushStack(strlen(expression)+1); //strlen does not count /0
    snprintf(line, sizeof(char) * lineSizeLimit, "STR(0x%x,%s); ", stackTop, expression);
    qLine();
    
    snprintf(line, sizeof(char) * lineSizeLimit, "CODE(%d)", statCodeCounter);
    qLine();

    statCodeCounter++;

    snprintf(line, sizeof(char) * lineSizeLimit, "R0=%d;", label);
    qLine();

    snprintf(line, sizeof(char) * lineSizeLimit, "R1=0x%x;", stackTop);
    qLine();

    qInstruction("GT(putf_);");

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



void qFinishWhile() {
    snprintf(line, sizeof(char) * lineSizeLimit, "GT(%d)", continueLabel);
    qLine();
    int auxLabel = label;
    label = breakLabel;
    newLabel();
    label = auxLabel;
}

void qLoadVar(int reg, char* identifier, enum RegType regType) {
    struct Reg* stEntry = searchRegType(identifier, regType);
    struct Reg* entryType = stEntry->typeReg;

    if (stEntry->value == NULL) {
        printf("Variable: %s. Value isn't initialized", identifier);
        return;
    }

    if (strcmp(entryType->regName, "int") == 0)
    {
        snprintf(line, sizeof(char) * lineSizeLimit, "R%d=I(0x%x);", reg, stEntry->value);
    } else if (strcmp(entryType->regName, "float") == 0)
    {
        snprintf(line, sizeof(char) * lineSizeLimit, "R%d=F(0x%x)", reg, stEntry->value);
    } else if (strcmp(entryType->regName, "char") == 0)
    {
        snprintf(line, sizeof(char) * lineSizeLimit, "R%d=U(0x%x)", reg, stEntry->value);
    } else if (strcmp(entryType->regName, "char*") == 0)
    {
        snprintf(line, sizeof(char) * lineSizeLimit, "R%d=P(0x%x)", reg, stEntry->value);
    }
    qLine();    
}

void qStore(char* identifier, int reg, enum RegType regType) {
    struct Reg* stEntry = searchRegType(identifier, regType);
    struct Reg* entryType = stEntry->typeReg;

    if (stEntry->value == NULL) {
        // assign some memory
    }

    if (strcmp(entryType->regName, "int") == 0)
    {
        snprintf(line, sizeof(char) * lineSizeLimit, "I(0x%x)=R%d;", stEntry->value, reg);
    } else if (strcmp(entryType->regName, "float") == 0)
    {
        snprintf(line, sizeof(char) * lineSizeLimit, "F(0x%x)=R%d", stEntry->value, reg);
    } else if (strcmp(entryType->regName, "char") == 0)
    {
        snprintf(line, sizeof(char) * lineSizeLimit, "U(0x%x)=R%d", stEntry->value, reg);
    } else if (strcmp(entryType->regName, "char*") == 0)
    {
        snprintf(line, sizeof(char) * lineSizeLimit, "P(0x%x)=R%d", stEntry->value, reg);
    }
    qLine();
}

void qLoadIntValue(int reg, int value) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=%d;", reg, value);
    qLine();
}

void qLoadFloatValue(int reg, float value) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=%f;", reg, value);
    qLine();
}

void qLoadCharValue(int reg, char value) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d='%c';", reg, value);
    qLine();
}

void qLoadStringValue(int reg, char* value) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=%p;", reg, value);
    qLine();
}

void qAdd(int reg1, int reg2) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d+R%d;", reg1, reg1, reg2);
    qLine();
}

void qSubtract(int reg1, int reg2) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d-R%d;", reg1, reg1, reg2);
    qLine();
}

void qMultiply(int reg1, int reg2) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d*R%d;", reg1, reg1, reg2);
    qLine();
}

void qDivide(int reg1, int reg2) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d/R%d;", reg1, reg1, reg2);
    qLine();
}

void qModulus(int reg1, int reg2) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d%%R%d;", reg1, reg1, reg2);
    qLine();
}

void qAnd(int reg1, int reg2) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d&&R%d", reg1, reg1, reg2);
    qLine();
}

void qOr(int reg1, int reg2) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d||R%d", reg1, reg1, reg2);
    qLine();
}

void qNegate(int reg1) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=!R%d", reg1, reg1);
    qLine();
}

void qEquals(int reg1, int reg2) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d==R%d", reg1, reg1, reg2);
    qLine();
}

void qNotEquals(int reg1, int reg2) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d!=R%d", reg1, reg1, reg2);
    qLine();
}

void qGreater(int reg1, int reg2) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d>R%d", reg1, reg1, reg2);
    qLine();
}

void qGreaterEquals(int reg1, int reg2) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d>=R%d", reg1, reg1, reg2);
    qLine();
}

void qLesser(int reg1, int reg2) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d<R%d", reg1, reg1, reg2);
    qLine();
}

void qLesserEquals(int reg1, int reg2) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d<=R%d", reg1, reg1, reg2);
    qLine();
}

void qReturn(int reg) {
    return;
}