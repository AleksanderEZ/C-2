#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "code_generation.h"
#include "symbol_table.h"

#define stackSize 10
#define notAssigned 0x24001

void yyerror(char* message);

int topElse = -1;
int topContinueBreak = -1;

FILE* obj;
int lineSizeLimit = 100;
int label = 0;
int atLabel = 0;
int continueLabel;
int breakLabel;
int elseLabel;
int skipElseLabel;
int statCodeCounter = 0;
int staticTop = 0x12000;
int stackBase = 0x9000;
int stackTop = 0x9000;
char* line;
int* registers;

int elseStack[stackSize];
int continueBreakStack[stackSize];

void push(int label, enum StackOption stackOption) {
    int* stack;
    int* top;
    switch (stackOption)
    {
    case CONTINUE_BREAK_STACK:
        stack = continueBreakStack;
        top = &topContinueBreak;
        break;
    case ELSE_STACK:
        stack = elseStack;
        top = &topElse;
        break;
    default:
        break;
    }

    if (*top = stackSize - 1) {
        yyerror("Stack full");
        return;
    }

    *top += 1;
    stack[*top] = label;
}

int pop(enum StackOption stackOption) {
    int* stack;
    int* top;
    switch (stackOption)
    {
    case CONTINUE_BREAK_STACK:
        stack = continueBreakStack;
        top = &topContinueBreak;
        break;
    case ELSE_STACK:
        stack = elseStack;
        top = &topElse;
        break;
    default:
        break;
    }

    if (*top == -1)
    {
        yyerror("Stack is empty!");
        return -1;
    }
    int returnValue = stack[*top];
    *top = *top - 1;
    return returnValue;
}

void pushStack(int bytes) {
    stackTop = stackTop - bytes;
}

void pushStatic(int bytes) {
    staticTop = staticTop - bytes;
}

void advanceLabel() {
    label++;
}

void newLabel() {
    if (atLabel == 1) {
        fprintf(obj, "\n");
        atLabel == 0;
    }
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
            return reg;
        }
    }
    yyerror("No regs available");
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
    fprintf(obj, "#");
    fprintf(obj, "include \"Q.h\"\n\nBEGIN\n");
    newLabel();
    advanceLabel();
}

void qEnd() {
    qInstruction("GT(__fin);");
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
        fprintf(obj, "    %s\n", line);
        atLabel = 0;
        return;
    }
    fprintf(obj, "        %s\n", line);
}

void qStat() {
    if (atLabel == 1) {
        fprintf(obj, "\n");
        atLabel = 0;
    }
    snprintf(line, sizeof(char) * lineSizeLimit, "STAT(%d)", statCodeCounter);
    fprintf(obj, "%s\n", line);
}

void qCode() {
    if (atLabel == 1) {
        fprintf(obj, "\n");
        atLabel = 0;
    }
    snprintf(line, sizeof(char) * lineSizeLimit, "CODE(%d)", statCodeCounter);
    fprintf(obj, "%s\n", line);
    statCodeCounter++;
}

void qCallFunction(char* function, char* arguments) {

}

void qCallFunctionNoArgs(char* function) {

}

void qMalloc(int reg) {

}

void qSizeOf(char* expression) {

}

void qStartPrint() {
    registers[0] = 1;
    registers[1] = 1;
    registers[2] = 1;
}

void qFinishPrint() {
    registers[0] = 0;
    registers[1] = 0;
    registers[2] = 0;
}

void qPrint(char* formatString, int reg) {
    qStat();

    pushStatic(strlen(formatString)+1); //strlen does not count /0
    snprintf(line, sizeof(char) * lineSizeLimit, "STR(0x%x,%s); ", staticTop, formatString);
    qLine();
    
    qCode();

    ////// SAVE REGS //////
    int r0clone = qAssignRegister();
    int r1clone = qAssignRegister();
    int r2clone = qAssignRegister();

    if(reg == 0) reg = r0clone;
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R0;", r0clone);
    qLine();

    if(reg == 1) reg = r1clone;
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R1;", r1clone);
    qLine();

    if(reg == 2) reg = r2clone;
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R2;", r2clone);
    qLine();

    //////  PRINT   //////
    snprintf(line, sizeof(char) * lineSizeLimit, "R0=%d;", label);
    qLine();

    snprintf(line, sizeof(char) * lineSizeLimit, "R1=0x%x;", staticTop);
    qLine();

    if (reg != -1) {
        snprintf(line, sizeof(char) * lineSizeLimit, "R2=R%d;", reg);
        qLine();
    }

    qInstruction("GT(putf_);");

    ///// RECOVER REGS //////
    
    snprintf(line, sizeof(char) * lineSizeLimit, "R0=R%d;", r0clone);
    qLine();

    snprintf(line, sizeof(char) * lineSizeLimit, "R1=R%d;", r1clone);
    qLine();

    snprintf(line, sizeof(char) * lineSizeLimit, "R2=R%d;", r2clone);
    qLine();
    
    newLabel();
    advanceLabel();

    qFreeRegister(r0clone);
    qFreeRegister(r1clone);
    qFreeRegister(r2clone);
}

void qPrintExplicit(char* expression) {
    qPrint(expression, -1);
}

void qPrintExplicitFormat(char* formatString, int reg) {
    qPrint(formatString, reg);
}

void qPrintImplicitFormat(char* identifier, int reg) {
    //identifier is string?
    struct Reg* result = search(identifier);
    if (strcmp(result->typeReg->regName, "char*") != 0) yyerror("Identifier not of string type");

    snprintf(line, sizeof(char) * lineSizeLimit, "R0=%d;", label);
    qLine();

    snprintf(line, sizeof(char) * lineSizeLimit, "R1=0x%x;", result->value);
    qLine();

    if (reg != -1) {
        snprintf(line, sizeof(char) * lineSizeLimit, "R2=R%d;", reg);
        qLine();
    }

    qInstruction("GT(putf_);");

    newLabel();
    advanceLabel();
}

void qStartWhile() {
    //continue label y break label en pila para habilitar anidamiento
    continueLabel = label;
    newLabel();
    advanceLabel();
    breakLabel = label;
    advanceLabel();
}

void qWhileCondition(int reg) {
    snprintf(line, sizeof(char) * lineSizeLimit, "IF(!R%d) GT(%d);", reg, breakLabel);
    qLine();
    qFreeRegister(reg);
}

void qFinishWhile() {
    snprintf(line, sizeof(char) * lineSizeLimit, "GT(%d);", continueLabel);
    qLine();
    int auxLabel = label;
    label = breakLabel;
    newLabel();
    label = auxLabel;
}

void qIf(int reg) {
    //pila de else label
    elseLabel = label;
    advanceLabel();
    snprintf(line, sizeof(char) * lineSizeLimit, "IF(!R%d) GT(%d);", reg, elseLabel);
    qLine();
}

void qElse() {
    int auxLabel = label;
    label = elseLabel;
    newLabel();
    label = auxLabel;
}

void qSkipElse() {
    skipElseLabel = label;
    advanceLabel();
    snprintf(line, sizeof(char) * lineSizeLimit, "GT(%d);", skipElseLabel);
    qLine();
}

void qSkipElseLabel() {
    int auxLabel = label;
    label = skipElseLabel;
    newLabel();
    label = auxLabel;
}

void qLoadVar(int reg, char* identifier) {
    struct Reg* result = search(identifier);
    if (result->type == localVariable) {
        // qLoadLocal();
    }
    if (result->type == globalVariable) {
        qLoadGlobal(reg, identifier);
    }
}

void qStoreVar(int reg, char* identifier) {
    struct Reg* result = search(identifier);
    if (result->type == localVariable) {
        // qStoreLocal();
    }
    if (result->type == globalVariable) {
        qStoreGlobal(reg, identifier);
    }
}

void qLoadGlobal(int reg, char* identifier) {
    struct Reg* stEntry = searchRegType(identifier, globalVariable);
    char* entryType = stEntry->typeReg->regName;
    int value = stEntry->value;

    if (value == notAssigned) {
        char* message = malloc(sizeof(char) * 40);
        snprintf(message, 40, "Variable: %s. Value isn't initialized", identifier);
        yyerror(message);
        free(message);
        return;
    }

    if (strcmp(entryType, "int") == 0)
    {
        snprintf(line, sizeof(char) * lineSizeLimit, "R%d=I(0x%x);", reg, value);
    } else if (strcmp(entryType, "float") == 0)
    {
        snprintf(line, sizeof(char) * lineSizeLimit, "R%d=F(0x%x);", reg, value);
    } else if (strcmp(entryType, "char") == 0)
    {
        snprintf(line, sizeof(char) * lineSizeLimit, "R%d=U(0x%x);", reg, value);
    } else if (strcmp(entryType, "char*") == 0)
    {
        snprintf(line, sizeof(char) * lineSizeLimit, "R%d=P(0x%x);", reg, value);
    }
    qLine();    
}

void qStoreGlobal(int reg, char* identifier) {
    struct Reg* stEntry = searchRegType(identifier, globalVariable);
    struct Reg* entryType = stEntry->typeReg;

    if (strcmp(entryType->regName, "int") == 0)
    {
        if (stEntry->value == notAssigned) {
            pushStatic(4);
            stEntry->value = staticTop;
        }
        snprintf(line, sizeof(char) * lineSizeLimit, "I(0x%x)=R%d;", stEntry->value, reg);
    } else if (strcmp(entryType->regName, "float") == 0)
    {
        if (stEntry->value == notAssigned) {
            pushStatic(4);
            stEntry->value = staticTop;
        }
        snprintf(line, sizeof(char) * lineSizeLimit, "F(0x%x)=R%d;", stEntry->value, reg);
    } else if (strcmp(entryType->regName, "char") == 0)
    {
        if (stEntry->value == notAssigned) {
            pushStatic(1);
            stEntry->value = staticTop;
        }
        snprintf(line, sizeof(char) * lineSizeLimit, "U(0x%x)=R%d;", stEntry->value, reg);
    } else if (strcmp(entryType->regName, "char*") == 0)
    {
        if (stEntry->value == notAssigned) {
            pushStatic(1);
            stEntry->value = staticTop;
        }
        snprintf(line, sizeof(char) * lineSizeLimit, "P(0x%x)=R%d;", stEntry->value, reg);
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
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d&&R%d;", reg1, reg1, reg2);
    qLine();
}

void qOr(int reg1, int reg2) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d||R%d;", reg1, reg1, reg2);
    qLine();
}

void qNegate(int reg1) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=!R%d;", reg1, reg1);
    qLine();
}

void qEquals(int reg1, int reg2) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d==R%d;", reg1, reg1, reg2);
    qLine();
}

void qNotEquals(int reg1, int reg2) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d!=R%d;", reg1, reg1, reg2);
    qLine();
}

void qGreater(int reg1, int reg2) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d>R%d;", reg1, reg1, reg2);
    qLine();
}

void qGreaterEquals(int reg1, int reg2) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d>=R%d;", reg1, reg1, reg2);
    qLine();
}

void qLesser(int reg1, int reg2) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d<R%d;", reg1, reg1, reg2);
    qLine();
}

void qLesserEquals(int reg1, int reg2) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d<=R%d;", reg1, reg1, reg2);
    qLine();
}

void qReturn(int reg) {
    return;
}