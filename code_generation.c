#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "code_generation.h"
#include "symbol_table.h"

#define stackSize 10
#define notAssigned 0x24001

extern int yylineno;
void yyerror(char* message);

char* currentFunction = NULL;

int currentValueList = notAssigned;
char* currentValueListType = NULL;
enum RegType currentValueListSwitch;
int currentValueListSize = 0;

int topElse = -1;
int topContinue = -1;
int topBreak = -1;
int topSkipElse = -1;
int topAdvance = -1;
int topParameter = -1;
int topSavedRegisters = -1;

int rClone;  //save R0, R1, R2 before using them in print

FILE* obj;
int lineSizeLimit = 100;
int label = 1;
int atLabel = 0;
int statCodeCounter = 0;
int staticTop = 0x12000;
int stackBase = 0xf000;
int stackAdvance;
char* line;
int* registers;

int skipElseStack[stackSize];
int elseStack[stackSize];
int continueStack[stackSize];
int breakStack[stackSize];
int advanceStack[stackSize];
int parameterStack[stackSize];
int savedRegistersStack[stackSize];

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
    case SAVED_REGISTERS_STACK:
        stack = savedRegistersStack;
        top = &topSavedRegisters;
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
    case SAVED_REGISTERS_STACK:
        stack = savedRegistersStack;
        top = &topSavedRegisters;
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

void qPopStack(int bytes) {
    snprintf(line, sizeof(char) * lineSizeLimit, "R7=R7+%d;", bytes);
    qLine();
    stackAdvance -= bytes;
}

void qFreeStack() {
    qInstruction("R7=R6;");
    stackAdvance = 0;
}

void qRecoverBase() {
    qInstruction("R6=P(R7+4);");
    if (topAdvance == -1) return;
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
    currentFunction = "main";
    int auxLabel = label;
    label = 0;
    newLabel();
    label = auxLabel;
    snprintf(line, sizeof(char) * lineSizeLimit, "R7=0x%x;", stackBase);
    qLine();
    
    qInstruction("R6=R7;");
}

int qFunctionDeclaration(char* functionName, int count, char** types, char** names) {
    int functionLabel = label;
    currentFunction = functionName;
    newLabel();
    advanceLabel();
    qNewStackBase();
    int paramSize = 8;

    for(int i = 0; i < count; i++) {
        char* type = types[i];
        char* name = names[i];
        struct Reg* var = searchRegType(name, parameter);
        var->value = paramSize * -1;
        paramSize = paramSize + qSizeOf(type);
    }

    return functionLabel;
}

void qFinishFunction() {
    qFreeStack();
    if (strcmp(currentFunction, "main") != 0) {
        qRecoverBase();
        qFunctionReturn();
    }
}

void qFunctionEnded() {
    currentFunction = NULL;
}

void qFunctionArguments(int reg) {
    push(reg, PARAMETER_STACK);
}

void qSaveAliveRegisters() {
    for (int i = 0; i < 6; i++)
    {
        if (registers[i] == 1)
        {
            qPushStack(qSizeOf("int"));
            snprintf(line, sizeof(char) * lineSizeLimit, "%c(R7)=R%d;", qTypeMnemonic("int"), i);
            qLine();
            push(i, SAVED_REGISTERS_STACK);
        }
    }
}

void qRecoverAliveRegs() {
    int poppedReg;
    while(topSavedRegisters > -1) {
        poppedReg = pop(SAVED_REGISTERS_STACK);
        snprintf(line, sizeof(char) * lineSizeLimit, "R%d=%c(R7);", poppedReg, qTypeMnemonic("int"));
        qLine();
        qPopStack(qSizeOf("int"));
    }
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
    if (strcmp(functionType, "void") == 0) {
        returnSize = 0;
    } else {
        returnSize = qSizeOf(functionType);
    }
    
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
        snprintf(line, sizeof(char) * lineSizeLimit, "%c(R7+%d)=R%d;", qTypeMnemonic(currentType), nextParameter, parameters[n]);
        qLine();
        newParameter(strdup(names[n]), searchRegType(types[n], type), yylineno);
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

    newLabel();
    advanceLabel();

    int reg = 0;
    if(returnSize > 0) {
        reg = qAssignRegister();
        snprintf(line, sizeof(char) * lineSizeLimit, "R%d=%c(R7+%d);", reg, qTypeMnemonic(functionType), paramSize);
        qLine();
    }

    snprintf(line, sizeof(char) * lineSizeLimit, "R7=R7+%d;", paramSize + returnSize);
    qLine();

    qRecoverAliveRegs();

    if(returnSize > 0) {
        return reg;
    } else {
        return 400;
    }
}

int qCallFunctionNoArgs(char* functionName) {
    struct Reg* searchResult = searchRegType(functionName, function);
    int functionLabel = searchResult->value;
    int functionParameters = searchResult->nParameters;
    int returnSize = 0;
    int paramSize = 8;

    int n = 0;
    struct Reg* param = searchResult;
    while(n < functionParameters) {
        param = param->next;
        paramSize = paramSize + qSizeOf(param->typeReg->regName);
        n = n + 1;
    }

    char* functionType = searchResult->typeReg->regName;
    if (strcmp(functionType, "void") == 0) {
        returnSize = 0;
    } else {
        returnSize = qSizeOf(functionType);
    }

    qPushStack(8 + returnSize);

    snprintf(line, sizeof(char) * lineSizeLimit, "P(R7+4)=R6;");
    qLine();

    snprintf(line, sizeof(char) * lineSizeLimit, "P(R7)=%d;", label);
    qLine();

    snprintf(line, sizeof(char) * lineSizeLimit, "GT(%d);", functionLabel);
    qLine();

    newLabel();
    advanceLabel();

    int reg = 0;
    if(returnSize > 0) {
        reg = qAssignRegister();
        snprintf(line, sizeof(char) * lineSizeLimit, "R%d=%c(R7+%d);", reg, qTypeMnemonic(functionType), paramSize);
        qLine();
    }

    snprintf(line, sizeof(char) * lineSizeLimit, "R7=R7+%d;", 8 + returnSize);
    qLine();

    if(returnSize > 0) {
        return reg;
    } else {
        return 400;
    }
}

void qReturn(int reg) {
    if (currentFunction == NULL) yyerror("Return with no function");
    struct Reg* searchResult = searchRegType(currentFunction, function);
    char* functionType = searchResult->typeReg->regName;
    if (reg == 400 && strcmp(functionType, "void") != 0) yyerror("Empty return in typed function");
    if (strcmp(functionType, "void") == 0) {
        qFunctionReturn();
        return;
    }
    int functionParameters = searchResult->nParameters;
    int paramSize = 8;

    int n = 0;
    struct Reg* param = searchResult;
    while(n < functionParameters) {
        param = param->next;
        paramSize = paramSize + qSizeOf(param->typeReg->regName);
        n = n + 1;
    }

    snprintf(line, sizeof(char) * lineSizeLimit, "%c(R6+%d)=R%d;", qTypeMnemonic(functionType), paramSize, reg);
    qLine();

    qFinishFunction();

    qFreeRegister(reg);
}

void qStartPrint(int reg) {
    for(int i = 0; i < 3; i++) {
        registers[i] = 1;
    }
    if (reg == 0) {
        rClone = qAssignRegister();
        snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R0;", rClone);
        qLine();
    } else if (reg == 1) {
        rClone = qAssignRegister();
        snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R1;", rClone);
        qLine();
    } else if (reg == 2) {
        rClone = qAssignRegister();
        snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R2;", rClone);
        qLine();
    }
}

void qFinishPrint(int reg) {
    qFreeRegister(rClone);
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d;", reg, rClone);
    qLine();
    if (reg != 0) qFreeRegister(0);
    if (reg != 1) qFreeRegister(1);
    if (reg != 2) qFreeRegister(2);
}

void qPrint(int address, int reg) {
    if (reg != -1) qStartPrint(reg);

    snprintf(line, sizeof(char) * lineSizeLimit, "R0=%d;", label);
    qLine();

    snprintf(line, sizeof(char) * lineSizeLimit, "R1=0x%x;", address);
    qLine();

    int originalReg = reg;
    if (reg != -1) {
        if (reg == 1 || reg == 2 || reg == 0) reg = rClone;
        snprintf(line, sizeof(char) * lineSizeLimit, "R2=R%d;", reg);
        qLine();
    }

    qInstruction("GT(putf_);");

    newLabel();
    advanceLabel();

    if (reg != -1) qFinishPrint(originalReg);
}

void qPrintFromReg(int regString, int regValue) {
    if (regValue != -1) qStartPrint(regValue);
    if (regString == 0) {
        snprintf(line, sizeof(char) * lineSizeLimit, "R1=R%d;", regString);
        qLine();
    }

    snprintf(line, sizeof(char) * lineSizeLimit, "R0=%d;", label);
    qLine();

    if (regString != 0) {
        snprintf(line, sizeof(char) * lineSizeLimit, "R1=R%d;", regString);
        qLine();
    }

    int originalReg = regValue;
    if (regValue != -1) {
        if (regValue == 1 || regValue == 2 || regValue == 0) regValue = rClone;
        snprintf(line, sizeof(char) * lineSizeLimit, "R2=R%d;", regValue);
        qLine();
    }

    qInstruction("GT(putf_);");

    newLabel();
    advanceLabel();

    if (regValue != -1) qFinishPrint(originalReg);
}

void qPrintExplicit(char* expression) {
    qStat();

    qPushStatic(strlen(expression)+1); //strlen does not count '/0'
    snprintf(line, sizeof(char) * lineSizeLimit, "STR(0x%x,%s); ", staticTop, expression);
    qLine();
    
    qCode();
    qPrint(staticTop, -1);
}

void qPrintExplicitFormat(char* formatString, int reg) {
    qStat();

    qPushStatic(strlen(formatString)+1); //strlen does not count '/0'
    snprintf(line, sizeof(char) * lineSizeLimit, "STR(0x%x,%s); ", staticTop, formatString);
    qLine();
    
    qCode();
    qPrint(staticTop, reg);
}

void qPrintImplicitFormat(char* identifier, int reg) {
    struct Reg* result = search(identifier);
    if (strcmp(result->typeReg->regName, "char*") != 0) yyerror("Identifier not of string type");
    int stringReg = qAssignRegister();
    qLoadVar(stringReg, identifier);
    qPrintFromReg(stringReg, reg);
    qFreeRegister(stringReg);
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
    if (result->type == localVariable || result->type == parameter) {
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
    if (result->type == localVariable || result->type == parameter) {
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

    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=%c(0x%x);", reg,  qTypeMnemonic(entryType), value);
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
    if (stEntry == NULL) stEntry = searchRegType(identifier, parameter);
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
    if (stEntry == NULL) stEntry = searchRegType(identifier, parameter);
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
    qStat();

    qPushStatic(strlen(value)+1); //strlen does not count '/0'
    snprintf(line, sizeof(char) * lineSizeLimit, "STR(0x%x,%s); ", staticTop, value);
    qLine();

    qCode();

    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=0x%x;", reg, staticTop);
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
    if(reg2 == -1) {
        snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d*%d;", reg1, reg1, -1);
        qLine();
        return;
    }
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
    if (strcmp(typeName, "int") == 0) {
        return 4;
    } else if (strcmp(typeName, "float") == 0)
    {
        return 4;
    } else if (strcmp(typeName, "char") == 0)
    {
        return 1;
    } else if (strcmp(typeName, "char*") == 0)
    {
        return 4;
    }
    yyerror("Unrecognized type");
}

char qTypeMnemonic(char* typeName) {
    if (strcmp(typeName, "int") == 0) {
        return 'I';
    } else if (strcmp(typeName, "float") == 0)
    {
        return 'F';
    } else if (strcmp(typeName, "char") == 0)
    {
        return 'U';
    } else if (strcmp(typeName, "char*") == 0)
    {
        return 'P';
    }
    yyerror("Unrecognized type");
}

void qReserveMemory(char* typeName, char* variableName, int slots, enum RegType variableSwitch) {
    int bytes = qSizeOf(typeName) * slots;
    if (variableSwitch == globalVariable) {
        struct Reg* result = searchRegType(variableName, globalVariable);
        qPushStatic(bytes);
        result->value = staticTop;
        result->arraySize = slots;
    } else if (variableSwitch == localVariable) {
        struct Reg* result = searchRegType(variableName, localVariable);
        qPushStack(bytes);
        result->value = stackAdvance;
        result->arraySize = slots;
    }
}

void qReserveArray(char* variableName) {
    struct Reg* result = searchRegType(variableName, currentValueListSwitch);
    if(result == NULL) searchRegType(variableName, parameter);
    result->value = currentValueList;
    result->arraySize = currentValueListSize-1;
    currentValueListType = NULL;
    currentValueList = notAssigned;
    currentValueListSize = 0;
}

void qExpandValueList(int reg) {
    if (currentValueListSwitch == globalVariable) {
        qExpandValueListStatic(reg);
    } else if (currentValueListSwitch == localVariable) {
        qExpandValueListStack(reg);
    }
}

void qExpandValueListStatic(int reg) {
    qPushStatic(qSizeOf(currentValueListType));
    snprintf(line, sizeof(char) * lineSizeLimit, "%c(0x%x)=R%d;", qTypeMnemonic(currentValueListType), currentValueList, reg);
    qLine();
    currentValueList = currentValueList + qSizeOf(currentValueListType);
    currentValueListSize = currentValueListSize + 1;
}

void qExpandValueListStack(int reg) {
    qPushStack(qSizeOf(currentValueListType));
    snprintf(line, sizeof(char) * lineSizeLimit, "%c(R7)=R%d;", qTypeMnemonic(currentValueListType), reg);
    qLine();
    currentValueList = currentValueList + qSizeOf(currentValueListType);
    currentValueListSize = currentValueListSize + 1;
}

void qNewValueList(char* type, enum RegType variableSwitch) {
    currentValueListSwitch = variableSwitch;
    if(variableSwitch == globalVariable) {
        qNewValueListStatic(type);
    } else if (variableSwitch == localVariable) {
        qNewValueListStack(type);
    }
}

void qNewValueListStatic(char* type) {
    if(currentValueList != notAssigned) yyerror("Creating value list while another is unfinished");
    if(currentValueListType != NULL) yyerror("Creating value list while another is unfinished");
    if(currentValueListSize != 0) yyerror("Creating value list while another is unfinished");
    int typeSize = qSizeOf(type);
    currentValueListType = type;
    qPushStatic(typeSize);
    currentValueList = staticTop;
}

void qNewValueListStack(char* type) {
    if(currentValueList != notAssigned) yyerror("Creating value list while another is unfinished");
    if(currentValueListSize != 0) yyerror("Creating value list while another is unfinished");
    if(currentValueListType != NULL) yyerror("Creating value list while another is unfinished");
    int typeSize = qSizeOf(type);
    currentValueListType = type;
    qPushStack(typeSize);
    currentValueList = stackAdvance;
}

void qArrayAccess(int reg, char* array, int regWithIndex) {
    struct Reg* result = search(array);
    if (result == NULL) {
        yyerror("Variable not declared");
    }
    if (result->type == localVariable || result->type == parameter) {
        qLocalArrayAccess(reg, array, regWithIndex);
    }
    if (result->type == globalVariable) {
        qGlobalArrayAccess(reg, array, regWithIndex);
    }
}

void qLocalArrayAccess(int reg, char* array, int regWithIndex) {
    struct Reg* arrayEntry = searchRegType(array, localVariable);
    if(arrayEntry == NULL) arrayEntry = searchRegType(array, parameter);
    char* arrayType = arrayEntry->typeReg->regName;
    int arrayLength = arrayEntry->arraySize;
    printf("hola%d\n", arrayLength);
    int value = arrayEntry->value;

    if(value > 0) snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R6-%d;", reg, value);
    if(value < 0) snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R6+%d;", reg, value);
    qLine();
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d+%d;", reg, reg, arrayLength * qSizeOf(arrayType));
    qLine();
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d*%d;", regWithIndex, regWithIndex, qSizeOf(arrayType));
    qLine();

    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d-R%d;", reg, reg, regWithIndex);
    qLine();

//    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=P(R%d);", reg, reg);
//    qLine();

    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=%c(R%d);", reg, qTypeMnemonic(arrayType), reg);
    qLine();
}

void qGlobalArrayAccess(int reg, char* array, int regWithIndex) {
    struct Reg* arrayFound = searchRegType(array, globalVariable);
    int value = arrayFound->value;
    int arrayLength = arrayFound->arraySize;
    char* arrayType = arrayFound->typeReg->regName;
    int freeReg = qAssignRegister();
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=0x%x;", freeReg, value);
    qLine();
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d+%d;", freeReg, freeReg, arrayLength * qSizeOf(arrayType));
    qLine();
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d*%d;", regWithIndex, regWithIndex, qSizeOf(arrayType));
    qLine();
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d-R%d;", freeReg, freeReg, regWithIndex);
    qLine();
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=%c(R%d);", reg, qTypeMnemonic(arrayType), freeReg);
    qLine();
    qFreeRegister(freeReg);
}

void qStoreArrayIndex(char* identifier, int regWithIndex, int regWithValue) {
    struct Reg* array = search(identifier);
    if (array == NULL) yyerror("Array not declared");
    if (array->type == globalVariable) {
        qStoreGlobalArrayIndex(identifier, regWithIndex,regWithValue);
    } else if (array->type == localVariable || array->type == parameter){
        qStoreLocalArrayIndex(identifier, regWithIndex, regWithValue);
    }
}

void qStoreLocalArrayIndex(char* identifier, int regWithIndex, int regWithValue) {
    struct Reg* array = searchRegType(identifier, localVariable);
    if (array == NULL) array = searchRegType(identifier, parameter);
    int value = array->value;
    int arrayLength = array->arraySize;
    char* arrayType = array->typeReg->regName;
    int freeReg = qAssignRegister();
    if (value > 0) snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R6-%d;", freeReg, value);
    if (value < 0) snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R6+%d;", freeReg, value);
    qLine();
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d+%d;", freeReg, freeReg, arrayLength * qSizeOf(arrayType));
    qLine();
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d*%d;", regWithIndex, regWithIndex, qSizeOf(arrayType));
    qLine();
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d-R%d;", freeReg, freeReg, regWithIndex);
    qLine();

    snprintf(line, sizeof(char) * lineSizeLimit, "%c(R%d)=R%d;", qTypeMnemonic(arrayType), freeReg, regWithValue);
    qLine();

    qFreeRegister(freeReg);
}

void qStoreGlobalArrayIndex(char* identifier, int regWithIndex, int regWithValue) {
    struct Reg* array = searchRegType(identifier, globalVariable);
    int value = array->value;
    char* arrayType = array->typeReg->regName;
    int arrayLength = array->arraySize;
    int freeReg = qAssignRegister();
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=0x%x;", freeReg, value);
    qLine();
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d+%d;", freeReg, freeReg, arrayLength * qSizeOf(arrayType));
    qLine();
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d*%d;", regWithIndex, regWithIndex, qSizeOf(arrayType));
    qLine();
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=R%d-R%d;", freeReg, freeReg, regWithIndex);
    qLine();
    snprintf(line, sizeof(char) * lineSizeLimit, "%c(R%d)=R%d;", qTypeMnemonic(arrayType), freeReg, regWithValue);
    qLine();
    qFreeRegister(freeReg);
}

void qLoadDefaultValue(char* identifier) {
    int reg = qAssignRegister();
    snprintf(line, sizeof(char) * lineSizeLimit, "R%d=0;", reg);
    qLine();
    qStoreVar(reg, identifier);
    qFreeRegister(reg);
}