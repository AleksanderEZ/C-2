#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "code_generation.h"
#include "symbol_table.h"

#define stackSize 10
#define notAssigned 0x24001

extern int yylineno;
void yyerror(char* message);

int topElse = -1;
int topContinue = -1;
int topBreak = -1;
int topSkipElse = -1;
int topAdvance = -1;
int topParameter = -1;

int r0clone;  //save R0, R1, R2 before using them in print
int r1clone;
int r2clone;

FILE* obj;
int lineSizeLimit = 100;
int label = 1;
int atLabel = 0;
int statCodeCounter = 0;
int staticTop = 0x12000;
int stackBase = 0x9000;
int stackAdvance;
char* line;
int* registers;

int skipElseStack[stackSize];
int elseStack[stackSize];
int continueStack[stackSize];
int breakStack[stackSize];
int advanceStack[stackSize];
int parameterStack[stackSize];

void push(int label, enum StackOption stackOption) {
    int* stack;
    int* top;
    switch (stackOption)
    {
    case CONTINUE_STACK:
        stack = continueStack;
        top = &topContinue;
        break;
    case ELSE_STACK:
        stack = elseStack;
        top = &topElse;
        break;
    case BREAK_STACK:
        stack = breakStack;
        top = &topBreak;
        break;
    case SKIP_ELSE_STACK:
        stack = skipElseStack;
        top = &topSkipElse;
        break;
    case ADVANCE_STACK:
        stack = advanceStack;
        top = &topAdvance;
        break;
    case PARAMETER_STACK:
        stack = parameterStack;
        top = &topParameter;
        break;
    default:
        break;
    }

    if (*top >= stackSize - 1) {
        yyerror("Stack full");
        return;
    }

    *top = *top + 1;
    stack[*top] = label;
}

int pop(enum StackOption stackOption) {
    int* stack;
    int* top;
    switch (stackOption)
    {
    case CONTINUE_STACK:
        stack = continueStack;
        top = &topContinue;
        break;
    case ELSE_STACK:
        stack = elseStack;
        top = &topElse;
        break;
    case BREAK_STACK:
        stack = breakStack;
        top = &topBreak;
        break;
    case SKIP_ELSE_STACK:
        stack = skipElseStack;
        top = &topSkipElse;
        break;
    case ADVANCE_STACK:
        stack = advanceStack;
        top = &topAdvance;
        break;
    case PARAMETER_STACK:
        stack = parameterStack;
        top = &topParameter;
        break;
    default:
        break;
    }

    if (*top <= -1)
    {
        yyerror("Stack is empty!");
        return -1;
    }
    int returnValue = stack[*top];
    *top = *top - 1;
    return returnValue;
}

void qNewStackBase() {
    push(stackAdvance, ADVANCE_STACK);
    qInstruction("R6=R7;");
    stackAdvance = 0;
}

void qPushStack(int bytes) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R7=R7-%d;", bytes);
    qLine();
    stackAdvance += bytes;
}

void qFreeStack() {
    qInstruction("R7=R6;");
    stackAdvance = 0;
}

void qRecoverBase() {
    qInstruction("R6=P(R7+4);");
    stackAdvance = pop(ADVANCE_STACK);
}

void qFunctionReturn() {
    int reg = qAssignRegister();
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=P(R7);", reg);
    qLine();
    snprintf(line, sizeof(char) * lineSizeLimit, "GT(R%d);", reg);
    qLine();
    qFreeRegister(reg);
}

void qPushStatic(int bytes) {
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
    for (int i = 0; i < 6; i++) {
        if(registers[i] == 0) {
            reg = i;
            registers[i] = 1;
            return reg;
        }
    }
    yyerror("No regs available");
}

void qFreeRegister(int reg) {
    if (reg >= 6) {
        yyerror("Attempted to access reg 6 or greater");
    }
    registers[reg] = 0;
}

void qFreeRegisters() {
    for (int i = 0; i < 6; i++) {
        registers[i] = 0;
    }
}

void qInit() {
    registers = malloc(sizeof(int) * 6);
    for (int i = 0; i < 6; i++) {
        registers[i] = 0;
    }
    line = malloc(sizeof(char) * lineSizeLimit);
    fprintf(obj, "#");
    fprintf(obj, "include \"Q.h\"\n\nBEGIN\n");
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

void qMain() {
    int auxLabel = label;
    label = 0;
    newLabel();
    label = auxLabel;
    snprintf(line, sizeof(char) * lineSizeLimit, "R7=0x%x;", stackBase);
    qLine();
    
    qInstruction("R6=R7;");
}

int qFunctionDeclaration(int count, char** types, char** names) {
    int functionLabel = label;
    newLabel();
    advanceLabel();
    qNewStackBase();
    int paramSize = 8;

    for(int i = 0; i < count; i++) {
        char* type = types[i];
        char* name = names[i];
        struct Reg* var = searchRegType(name, localVariable);
        var->value = paramSize * -1;
        paramSize = paramSize + qSizeOf(type);
    }

    return functionLabel;
}

void qFinishFunction() {
    qFreeStack();
    qRecoverBase();
    qFunctionReturn();
}

void qFunctionArguments(int reg) {
    push(reg, PARAMETER_STACK);
}

int qCallFunction(char* functionName) {
    struct Reg* searchResult = searchRegType(functionName, function);
    int functionLabel = searchResult->value;
    int functionParameters = searchResult->nParameters;
    int paramSize = 8;
    int returnSize = 0;

    int n = 0;
    struct Reg* param = searchResult;
    char* types[functionParameters];
    char* names[functionParameters];
    while(n < functionParameters) {
        param = param->next;
        paramSize = paramSize + qSizeOf(param->typeReg->regName);
        types[n] = param->typeReg->regName;
        names[n] = param->regName;
        n = n + 1;
    }

    char* functionType = searchResult->typeReg->regName;
    returnSize = qSizeOf(functionType);
    
    qPushStack(paramSize + returnSize);

    int regs = topParameter+1;
    n = topParameter;
    int parameters[regs];
    while(n > -1) {
        parameters[n] = pop(PARAMETER_STACK);
        n = n - 1;
    }

    int nextParameter = 8;
    n = 0;
    while(n < regs) {
        char* currentType = types[n];
        snprintf(line, sizeof(char) * lineSizeLimit, "%c(R7+%d)=R%d", qTypeMnemonic(currentType), nextParameter, parameters[n]);
        qLine();
        newParameter(strdup(names[n]), localVariable, searchRegType(types[n], type), yylineno);
        nextParameter = nextParameter + qSizeOf(currentType);
        qFreeRegister(parameters[n]);
        n = n + 1;
    }

    snprintf(line, sizeof(char) * lineSizeLimit, "P(R7+4)=R6;");
    qLine();

    snprintf(line, sizeof(char) * lineSizeLimit, "P(R7)=%d;", label);
    qLine();

    snprintf(line, sizeof(char) * lineSizeLimit, "GT(%d);", functionLabel);
    qLine();

    int reg = qAssignRegister();
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=%c(R7+%d)", reg, qTypeMnemonic(functionType), paramSize);
    qLine();

    newLabel();
    advanceLabel();
    snprintf(line, sizeof(char) * lineSizeLimit, "R7=R7+%d;", paramSize + returnSize);
    qLine();

    return reg;
}

int qCallFunctionNoArgs(char* functionName) {
    struct Reg* searchResult = searchRegType(functionName, function);
    int functionLabel = searchResult->value;
    int returnSize = 0;

    char* functionType = searchResult->typeReg->regName;
    returnSize = qSizeOf(functionType);

    qPushStack(8 + returnSize);

    snprintf(line, sizeof(char) * lineSizeLimit, "P(R7+4)=R6;");
    qLine();

    snprintf(line, sizeof(char) * lineSizeLimit, "P(R7)=%d;", label);
    qLine();

    snprintf(line, sizeof(char) * lineSizeLimit, "GT(%d);", functionLabel);
    qLine();

    newLabel();
    advanceLabel();
    snprintf(line, sizeof(char) * lineSizeLimit, "R7=R7+%d;", 8 + returnSize);
    qLine();
}

void qReturn(int reg) {
    // What is the type of reg
    // What is the parameterSize of the function
    snprintf(line, sizeof(char) * lineSizeLimit, "type(R6+parameterSize)=R%d", reg);
    qLine();
    qFreeRegister(reg);
}

void qStartPrint() {
    if(registers[0] == 1) {
        r0clone = qAssignRegister();
        snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R0;", r0clone);
        qLine();
    } else {
        registers[0] = 1;
    }

    if(registers[1] == 1) {
        r1clone = qAssignRegister();
        snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R1;", r1clone);
        qLine();
    } else {
        registers[1] = 1;
    }

    if(registers[2] == 1) {
        r2clone = qAssignRegister();
        snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R2;", r2clone);
        qLine();
    } else {
        registers[2] = 1;
    }
}

void qFinishPrint() {
    if(registers[r0clone] == 1) { 
        snprintf(line, sizeof(char) * lineSizeLimit, "R0=R%d;", r0clone);
        qLine();
        qFreeRegister(r0clone);
    }
    if(registers[r1clone] == 1) { 
        snprintf(line, sizeof(char) * lineSizeLimit, "R1=R%d;", r1clone);
        qLine();
        qFreeRegister(r1clone);
    }

    if(registers[r2clone] == 1) { 
        snprintf(line, sizeof(char) * lineSizeLimit, "R2=R%d;", r2clone);
        qLine();
        qFreeRegister(r2clone);
    }
}

void qPrint(char* formatString, int reg) {
    qStat();

    qPushStatic(strlen(formatString)+1); //strlen does not count '/0'
    snprintf(line, sizeof(char) * lineSizeLimit, "STR(0x%x,%s); ", staticTop, formatString);
    qLine();
    
    qCode();

    snprintf(line, sizeof(char) * lineSizeLimit, "R0=%d;", label);
    qLine();

    snprintf(line, sizeof(char) * lineSizeLimit, "R1=0x%x;", staticTop);
    qLine();

    if (reg != -1) {
        snprintf(line, sizeof(char) * lineSizeLimit, "R2=R%d;", reg);
        qLine();
    }

    qInstruction("GT(putf_);");

    newLabel();
    advanceLabel();
}

void qPrintExplicit(char* expression) {
    qPrint(expression, -1);
}

void qPrintExplicitFormat(char* formatString, int reg) {
    qPrint(formatString, reg);
}

void qPrintImplicitFormat(char* identifier, int reg) {
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
    push(label, CONTINUE_STACK);
    newLabel();
    advanceLabel();
    push(label, BREAK_STACK);
    advanceLabel();
}

void qWhileCondition(int reg) {
    int breakLabel = breakStack[topBreak];
    snprintf(line, sizeof(char) * lineSizeLimit, "IF(!R%d) GT(%d);", reg, breakLabel);
    qLine();
    qFreeRegister(reg);
}

void qFinishWhile() {
    int continueLabel = pop(CONTINUE_STACK);
    snprintf(line, sizeof(char) * lineSizeLimit, "GT(%d);", continueLabel);
    qLine();
    int auxLabel = label;
    label = pop(BREAK_STACK);
    newLabel();
    label = auxLabel;
}

void qIf(int reg) {
    int elseLabel = label;
    advanceLabel();
    snprintf(line, sizeof(char) * lineSizeLimit, "IF(!R%d) GT(%d);", reg, elseLabel);
    qLine();
    push(elseLabel, ELSE_STACK);
}

void qElse() {
    int auxLabel = label;
    label = pop(ELSE_STACK);
    newLabel();
    label = auxLabel;
}

void qSkipElse() {
    int skipElseLabel = label;
    advanceLabel();
    snprintf(line, sizeof(char) * lineSizeLimit, "GT(%d);", skipElseLabel);
    qLine();
    push(skipElseLabel, SKIP_ELSE_STACK);
}

void qSkipElseLabel() {
    int auxLabel = label;
    label = pop(SKIP_ELSE_STACK);
    newLabel();
    label = auxLabel;
}

void qLoadVar(int reg, char* identifier) {
    struct Reg* result = search(identifier);
    if (result == NULL) {
        yyerror("Variable not declared");
    }
    if (result->type == localVariable) {
        qLoadLocal(reg, identifier);
    }
    if (result->type == globalVariable) {
        qLoadGlobal(reg, identifier);
    }
}

void qStoreVar(int reg, char* identifier) {
    struct Reg* result = search(identifier);
    if (result == NULL) {
        yyerror("Variable not declared");
    }
    if (result->type == localVariable) {
        qStoreLocal(reg, identifier);
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

    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=%c(0x%x);", reg, qTypeMnemonic(entryType), value);
    qLine();    
}

void qStoreGlobal(int reg, char* identifier) {
    struct Reg* stEntry = searchRegType(identifier, globalVariable);
    struct Reg* entryType = stEntry->typeReg;

    if (stEntry->value == notAssigned) {
        qPushStatic(qSizeOf(entryType->regName));
        stEntry->value = staticTop;
    }
    snprintf(line, sizeof(char) * lineSizeLimit, "%c(0x%x)=R%d;", qTypeMnemonic(entryType->regName), stEntry->value, reg);
    qLine();
}

void qLoadLocal(int reg, char* identifier) {
    struct Reg* stEntry = searchRegType(identifier, localVariable);
    char* entryType = stEntry->typeReg->regName;
    int value = stEntry->value;

    if (value == notAssigned) {
        char* message = malloc(sizeof(char) * 40);
        snprintf(message, 40, "Variable: %s. Value isn't initialized", identifier);
        yyerror(message);
        free(message);
        return;
    }

    if (value > 0) snprintf(line, sizeof(char) * lineSizeLimit, "R%d=%c(R6-%d);", reg, qTypeMnemonic(entryType), value);
    if (value < 0) snprintf(line, sizeof(char) * lineSizeLimit, "R%d=%c(R6+%d);", reg, qTypeMnemonic(entryType), abs(value));
    qLine();    
}

void qStoreLocal(int reg, char* identifier) {
    struct Reg* stEntry = searchRegType(identifier, localVariable);
    struct Reg* entryType = stEntry->typeReg;

    if (stEntry->value == notAssigned) {
        qPushStack(qSizeOf(entryType->regName));
        stEntry->value = stackAdvance;
    }
    snprintf(line, sizeof(char) * lineSizeLimit, "%c(R6-%d)=R%d;", qTypeMnemonic(entryType->regName), stEntry->value, reg);
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

int qSizeOf(char* typeName) {
    if (strcmp(typeName, "int")) {
        return 4;
    } else if (strcmp(typeName, "float"))
    {
        return 4;
    } else if (strcmp(typeName, "char"))
    {
        return 1;
    } else if (strcmp(typeName, "char*"))
    {
        return 4;
    }
    yyerror("Unrecognized type");
}

char qTypeMnemonic(char* typeName) {
    if (strcmp(typeName, "int")) {
        return 'I';
    } else if (strcmp(typeName, "float"))
    {
        return 'F';
    } else if (strcmp(typeName, "char"))
    {
        return 'U';
    } else if (strcmp(typeName, "char*"))
    {
        return 'P';
    }
    yyerror("Unrecognized type");
}